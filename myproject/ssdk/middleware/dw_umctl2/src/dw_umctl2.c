/********************************************************
 *	        Copyright(c) 2020	Semidrive 		        *
 *******************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <debug.h>
#include <reg.h>
#include <regs_base.h>
#include <string.h>
#include <dw_umctl2.h>
#include <ddr_init_helper.h>
#include <board.h>

extern const char *find_streaming_string(uint32_t fw, uint32_t id);

extern uint8_t ddrphy_fw_lpddr4x_dmem[];
extern uint8_t ddrphy_fw_lpddr4x_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4x_imem[];
extern uint8_t ddrphy_fw_lpddr4x_imem_end[];
extern uint8_t ddrphy_fw_lpddr4x_2d_dmem[];
extern uint8_t ddrphy_fw_lpddr4x_2d_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4x_2d_imem[];
extern uint8_t ddrphy_fw_lpddr4x_2d_imem_end[];

extern uint8_t ddrphy_fw_lpddr4_dmem[];
extern uint8_t ddrphy_fw_lpddr4_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4_imem[];
extern uint8_t ddrphy_fw_lpddr4_imem_end[];
extern uint8_t ddrphy_fw_lpddr4_2d_dmem[];
extern uint8_t ddrphy_fw_lpddr4_2d_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4_2d_imem[];
extern uint8_t ddrphy_fw_lpddr4_2d_imem_end[];

extern uint8_t ddrphy_fw_ddr4_dmem[];
extern uint8_t ddrphy_fw_ddr4_dmem_end[];
extern uint8_t ddrphy_fw_ddr4_imem[];
extern uint8_t ddrphy_fw_ddr4_imem_end[];
extern uint8_t ddrphy_fw_ddr4_2d_dmem[];
extern uint8_t ddrphy_fw_ddr4_2d_dmem_end[];
extern uint8_t ddrphy_fw_ddr4_2d_imem[];
extern uint8_t ddrphy_fw_ddr4_2d_imem_end[];

typedef struct {
    uint32_t addr;
    uint32_t val;
    uint32_t val_ecc;
}pre_init_reg_t;

typedef struct {
    uint32_t size_MB;
    bool ecc_en;
    pre_init_reg_t preinit_regs[100];
    uint32_t preinit_regs_num;
}ddr_type_t;

static ddr_type_t ddr_type;

extern void *ddrphy_apb_memcpy(uint32_t *to, uint16_t *from, size_t sz);

static uint32_t ddr_get_data_field(uint32_t data, uint32_t offset, uint32_t width)
{
    return ((data << (32 - offset - width)) >> (32 - width));
}

static uint32_t ddrc_rd_bits(uint32_t addr, uint32_t offset, uint32_t width)
{
    uint32_t v = readl(addr);
    return ddr_get_data_field(v, offset, width);
}

static inline void ddrc_wr_bits(uint32_t addr, uint32_t offset, uint32_t width, uint32_t v)
{
    RMWREG32(addr, offset, width, v);
}

static inline void ddrc_wr(uint32_t addr, uint32_t v)
{
    writel(v, addr);
}

static unsigned char max_abs_4(signed char A, signed char B, signed char C, signed char D)
{
     unsigned char A1;
     unsigned char B1;
     unsigned char C1;
     unsigned char D1;
     unsigned char max;

     A1 = (A>0)?A:(-1*A);
     B1 = (B>0)?B:(-1*B);
     C1 = (C>0)?C:(-1*C);
     D1 = (D>0)?D:(-1*D);

     max = A1;
     max = (B1>=max)?B1:max;
     max = (C1>=max)?C1:max;
     max = (D1>=max)?D1:max;

     return max;
}

static int32_t max_abs_4int(int A, int B, int C, int D)
{
     int max;

     max = A;
     max = (B>=max)?B:max;
     max = (C>=max)?C:max;
     max = (D>=max)?D:max;

     return max;
}

static unsigned char max_abs_2(signed char A, signed char B)
{
     unsigned char A1;
     unsigned char B1;
     unsigned char max;

     A1 = (A>0)?A:(-1*A);
     B1 = (B>0)?B:(-1*B);

     max = A1;
     max = (B1>=max)?B1:max;

     return max;
}

static unsigned char max2(unsigned char A, unsigned char B)
{
     unsigned char max;

     max = A;
     max = (B>=max)?B:max;

     return max;
}

static int dwc_ddrphy_apb_rd(int phy_addr)
{
    return readl(APB_DDRPHY_BASE + phy_addr * 4);
}

static void dwc_ddrphy_apb_wr(int phy_addr, int phy_data)
{
    int w_addr = APB_DDRPHY_BASE + phy_addr * 4;
    int w_data = phy_data;
    writel(w_data, w_addr);
}

static int dwc_ddrphy_get_mail(int mode)
{
    int mail;
    while((0x1 & dwc_ddrphy_apb_rd(0x000d0004)) != 0);  //UctWriteProtShadow

    mail = dwc_ddrphy_apb_rd(0x000d0032);

    if(mode==32) {
        mail |= (dwc_ddrphy_apb_rd(0x000d0034)<<16);
    }

    dwc_ddrphy_apb_wr(0x000d0031, 0);
    while((0x1&dwc_ddrphy_apb_rd(0x000d0004)) == 0) {;} //UctWriteProtShadow
    dwc_ddrphy_apb_wr(0x000d0031, 1);
    return mail;
}

static const char* ddrphy_decode_major_message(int major_info)
{
    return (major_info == 0x00 ? "End of initialization" :
            major_info == 0x01 ? "End of fine write leveling" :
            major_info == 0x02 ? "End of read enable training" :
            major_info == 0x03 ? "End of read delay center optimization" :
            major_info == 0x04 ? "End of write dealy center optimization" :
            major_info == 0x05 ? "End of 2D read delay/voltage center optimization" :
            major_info == 0x06 ? "End of 2D write delay/voltage center optimization" :
            major_info == 0x07 ? "Training has run successfully":
            major_info == 0x08 ? "Streaming message":
            major_info == 0x09 ? "End of max read latency training" :
            major_info == 0x0a ? "End of read dq deskew training" :
            major_info == 0x0b ? "Reserved" :
            major_info == 0x0c ? "End of all DB training" :
            major_info == 0x0d ? "End of CA training" :
            major_info == 0xfd ? "End of MPR read delay center optimization" :
            major_info == 0xfe ? "End of Write leveling coarse delay" :
            major_info == 0xff ? "Training has failed" : "NOT defined major message");
}

static volatile int para[32];
static void parse_streaming_msg(uint32_t fw)
{
    int string_id = dwc_ddrphy_get_mail(32);
    int para_N = string_id & 0xffff;

    if (para_N > 32) {
        para_N = 32;
    }

    for(int i=0; i<para_N; i++) {
        para[i] = dwc_ddrphy_get_mail(32);
    }

#if defined(DDR_DEBUG)
    const char *fmt = find_streaming_string(fw, string_id);
    switch(para_N) {
    case 0:
        DDR_DBG(fmt);
        break;
    case 1:
        DDR_DBG(fmt, para[0]);
        break;
    case 2:
        DDR_DBG(fmt, para[0], para[1]);
        break;
    case 3:
        DDR_DBG(fmt, para[0], para[1], para[2]);
        break;
    case 4:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3]);
        break;
    case 5:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4]);
        break;
    case 6:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5]);
        break;
    case 7:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6]);
        break;
    case 8:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7]);
        break;
    case 9:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                    para[8]);
        break;
    case 10:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                    para[8], para[9]);
        break;
    case 11:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                    para[8], para[9], para[10]);
        break;
    case 12:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                    para[8], para[9], para[10], para[11]);
        break;
    case 31:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                    para[8], para[9], para[10], para[11], para[12], para[13], para[14], para[15],
                    para[16], para[17], para[18], para[19], para[20], para[21], para[22], para[23],
                    para[24], para[25], para[26], para[27], para[28], para[29], para[30]);
        break;
    default:
        DDR_DBG("%s: %d paras, not supported yet\n", __FUNCTION__, para_N);
        break;
    }
#endif  /* defined(DDR_DEBUG) */
}

static uint8_t ddrmrr(uint32_t mr_num, uint32_t rank_bits)
{
    unsigned int  time_out = 5000;
    // exception
    writel(1,APB_DDRCTRL_BASE+0x00020060);
    //Poll MRSTAT.mr_wr_busy until it is 0
    //0 mr_wr_busy : The Soc might initiate a MR write operation only if this signal is low
    do{
        udelay(1);
    }while((readl(APB_DDRCTRL_BASE+UMCTL2_REGS_MRSTAT_OFF) & BM_MRSTAT_MR_WR_BUSY) && time_out--);
    if(!time_out){
        DDR_DBG("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }

    writel(((mr_num & 0xFF)<<8),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL1_OFF);
    writel((0<<31)+(0<<30)+((mr_num&0xF)<<12)+(rank_bits<<4)+(1<<0),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL0_OFF);
    writel((1<<31)+(0<<30)+((mr_num&0xF)<<12)+(rank_bits<<4)+(1<<0),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL0_OFF);
    //Poll MRSTAT.mr_wr_busy until it is 0
    //0 mr_wr_busy : The Soc might initiate a MR write operation only if this signal is low
    time_out = 5000;
    do{
        udelay(1);
    }while((!(readl(APB_DDRCTRL_BASE+UMCTL2_REGS_MRSTAT_OFF)&BM_MRSTAT_MR_WR_BUSY)) && time_out--);
    if(!time_out){
        DDR_DBG("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }
    // poll avild
    time_out = 5000;
    do{
        udelay(1);
    }while(((readl(APB_DDRCTRL_BASE+0x00020060) & 0x8) != 0x8) && time_out--);
    if(!time_out){
        DDR_DBG("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }

    return (readl(APB_DDRCTRL_BASE+0x00020064)>>8) & 0xFF;
}

static uint32_t ddr_get_size(void)
{
    uint8_t mr8;
    mr8 = ddrmrr(8, 1);
    if (mr8 == 0x18) {
        return 8 * 1024;
    } else if (mr8 == 0x10) {
        return 4 * 1024;
    } else if (mr8 == 0x8) {
        return 2 * 1024;
    } else {
        return 1 * 1024;
    }
}

static void __ddr_get_ecc(void)
{
    uint32_t value = readl(APB_DDRCTRL_BASE + UMCTL2_REGS_ECCCTL_OFF);

    if (value == 0x700)
        ddr_type.ecc_en =  true;
    else
        ddr_type.ecc_en = false;
}

static bool ddr_get_ecc(void)
{
    static bool geted = false;

    if (!geted) {
        __ddr_get_ecc();
        geted = true;
    }

    return ddr_type.ecc_en;
}

static void add_pre_init_reg(uint32_t addr, uint32_t val, uint32_t val_ecc)
{
    if (ddr_type.preinit_regs_num >= 100) {
        DDR_DBG("oops,  too many preinit regs\n");
        return;
    }

    ddr_type.preinit_regs[ddr_type.preinit_regs_num].addr = addr;
    ddr_type.preinit_regs[ddr_type.preinit_regs_num].val = val;
    ddr_type.preinit_regs[ddr_type.preinit_regs_num].val_ecc = val_ecc;
    ddr_type.preinit_regs_num++;
}

void ddr_get_type(void)
{
    if (ddr_type.size_MB == 0)
        ddr_type.size_MB = ddr_get_size();

    if (ddr_type.size_MB == 8 * 1024) {
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP0_OFF, 0x18, 0x15);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP1_OFF, 0x50505, 0x50505);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP2_OFF, 0x0, 0x2020202);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP3_OFF, 0x3030300, 0x17171702);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP4_OFF, 0x1f1f, 0x80001f1f);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP5_OFF, 0x70f0707, 0x40f0404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP6_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP7_OFF, 0xf07, 0xf04);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP8_OFF, 0x0, 0x0);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP9_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP10_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP11_OFF, 0x7, 0x4);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_RFSHTMG_OFF, 0x82012c, 0x82012c);
    } else if (ddr_type.size_MB == 4 * 1024) {
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP0_OFF, 0x17, 0x14);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP1_OFF, 0x50505, 0x50505);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP2_OFF, 0x0, 0x2020202);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP3_OFF, 0x3030300, 0x16161602);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP4_OFF, 0x1f1f, 0x80001f1f);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP5_OFF, 0x70f0707, 0x40f0404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP6_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP7_OFF, 0xf0f, 0xf0f);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP8_OFF, 0x0, 0x0);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP9_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP10_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP11_OFF, 0x7, 0x4);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_RFSHTMG_OFF, 0x8200c1, 0x8200c1);
    } else if (ddr_type.size_MB == 2 * 1024) {
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP0_OFF, 0x1f, 0x1f);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP1_OFF, 0x80808, 0x50505);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP2_OFF, 0x0, 0x2020202);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP3_OFF, 0x0, 0x15151502);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP4_OFF, 0x1f1f, 0x80001f1f);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP5_OFF, 0x70f0707, 0x40f0404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP6_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP7_OFF, 0xf0f, 0xf0f);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP8_OFF, 0x0, 0x0);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP9_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP10_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP11_OFF, 0x7, 0x4);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_MSTR_OFF, 0x81080020, 0x81080020);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_RFSHTMG_OFF, 0x610068, 0x610068);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_RFSHTMG1_OFF, 0x300000, 0x300000);
        add_pre_init_reg(APB_DDRPHY_BASE+0x54012*4, 0x110, 0x110);
        add_pre_init_reg(APB_DDRPHY_BASE+0x5402c*4, 0x1, 0x1);
    } else if (ddr_type.size_MB == 1 * 1024) {
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP0_OFF, 0x15, 0x12);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP1_OFF, 0x80808, 0x50505);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP2_OFF, 0x0, 0x2020202);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP3_OFF, 0x0, 0x14141402);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP4_OFF, 0x1f1f, 0x80001f1f);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP5_OFF, 0x70f0707, 0x40f0404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP6_OFF, 0xf0f0707, 0xf0f0404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP7_OFF, 0xf0f, 0xf0f);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP8_OFF, 0x0, 0x0);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP9_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP10_OFF, 0x7070707, 0x4040404);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_ADDRMAP11_OFF, 0x7, 0x4);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_MSTR_OFF, 0x83080020, 0x83080020);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_RFSHTMG_OFF, 0x6100e0, 0x6100e0);
        add_pre_init_reg(APB_DDRCTRL_BASE+UMCTL2_REGS_RFSHTMG1_OFF, 0x480000, 0x480000);
        add_pre_init_reg(APB_DDRPHY_BASE+0x54012*4, 0x310, 0x310);
        add_pre_init_reg(APB_DDRPHY_BASE+0x5402c*4, 0x3, 0x3);
    }
}

void set_preinit_reg(uint32_t addr)
{
    uint32_t i;
    bool ecc = ddr_get_ecc();
    for(i = 0; i < ddr_type.preinit_regs_num; i++) {
        if (ddr_type.preinit_regs[i].addr == addr) {
            if (ecc)
                writel(ddr_type.preinit_regs[i].val_ecc, addr);
            else
                writel(ddr_type.preinit_regs[i].val, addr);
        }
    }
}

void kick_scrubber(uint32_t column_start, uint32_t column_end, uint32_t value)
{
    uint32_t base_addr=APB_DDRCTRL_BASE;
    int cnt= (column_end - column_start)/4096 + 2;

    writel(value, base_addr+0xf2c);
    writel(column_start, base_addr+0xf38);
    writel(column_end, base_addr+0xf40);
    writel(0xff14, base_addr+0xf24);
    writel(0x14, base_addr+0xf24);
    writel(0x15, base_addr+0xf24);

    while((readl(base_addr+0xf28) & 0x2) == 0 && cnt--)
    {
        udelay(10);
    }
    if(cnt <= 0) {
        printf("init ddr timeout\n");
        return;
    }
    writel(0x14, base_addr+0xf24);

}

void init_ddr_by_hw(uint32_t start_addr, uint32_t sizeKB, uint32_t value, const int column_step)
{
    uint32_t base_addr=APB_DDRCTRL_BASE;
    uint32_t column_start,column_end;

    column_start=start_addr>>column_step;
    column_end = (((sizeKB)>>column_step) * 1024)-1 + column_start;
    //INFO("columns start 0x%x end 0x%x init ddr size %d KB\n", column_start, column_end, sizeKB);
    writel(0, base_addr+0x490);
    kick_scrubber(column_start, column_end, value);
    writel(1, base_addr+0x490);
}

void memset_by_hw(uint32_t start_addr, uint32_t value, uint32_t size)
{
    uint32_t column_start, column_end;

    column_start = (start_addr - 0x30000000) >> 2;
    column_end = (size >> 2) - 1 + column_start;
    kick_scrubber(column_start, column_end, value);
}

void check_ecc_clr_memory(void)
{
    uint32_t clr_size;
    if (ddr_get_ecc()) {
        if (ddr_type.size_MB == 0)
            ddr_type.size_MB = ddr_get_size();

        clr_size = (ddr_type.size_MB >> 3) * 7;

        init_ddr_by_hw(0, (clr_size * 1024), 0xbad0bad0, 2);
    }
}

uint32_t dwc_ddrphy_fw_monitor_loop(uint32_t fw)
{
    int major_info;

    DDR_DBG("Entering fw mointor loop...\n");
    while(1) {
        major_info = dwc_ddrphy_get_mail(16);
        if (0x08u == major_info) {  /* Streaming message */
            parse_streaming_msg(fw);
        } else {
            DDR_DBG("ddrphy fw mail:0x%x:%s\n", major_info,
                    ddrphy_decode_major_message(major_info));
        }
        if( (major_info == 0x07) || (major_info == 0xff)) {
            break;
        }
    }

    if(major_info == 0xff) {
        printf("%s: Opps: DDR Training Failed.\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

void *ddrphy_apb_memcpy(uint32_t *to, uint16_t *from, size_t sz)
{
    uint32_t v = 0;
    void *p = (void*)to;

    assert((NULL != to) && (NULL != from) && (sz % 2 == 0));

    while(sz) {
        v = *from;
        *to = (uint16_t)v;
        from++;
        to++;
        sz -= 2;
    }
    return p;
}

/* variables to save training result then update to ctrl regs */
static signed char CDD_ChA_RR_1_0;
static signed char CDD_ChA_RR_0_1;
static signed char CDD_ChA_RW_1_1;
static signed char CDD_ChA_RW_1_0;
static signed char CDD_ChA_RW_0_1;
static signed char CDD_ChA_RW_0_0;
static signed char CDD_ChA_WR_1_1;
static signed char CDD_ChA_WR_1_0;
static signed char CDD_ChA_WR_0_1;
static signed char CDD_ChA_WR_0_0;
static signed char CDD_ChA_WW_1_0;
static signed char CDD_ChA_WW_0_1;

static signed char CDD_ChB_RR_1_0;
static signed char CDD_ChB_RR_0_1;
static signed char CDD_ChB_RW_1_1;
static signed char CDD_ChB_RW_1_0;
static signed char CDD_ChB_RW_0_1;
static signed char CDD_ChB_RW_0_0;
static signed char CDD_ChB_WR_1_1;
static signed char CDD_ChB_WR_1_0;
static signed char CDD_ChB_WR_0_1;
static signed char CDD_ChB_WR_0_0;
static signed char CDD_ChB_WW_1_0;
static signed char CDD_ChB_WW_0_1;

/* variables for DDR4 */
static signed char CDD_RR_3_2;
static signed char CDD_RR_3_1;
static signed char CDD_RR_3_0;
static signed char CDD_RR_2_3;
static signed char CDD_RR_2_1;
static signed char CDD_RR_2_0;
static signed char CDD_RR_1_3;
static signed char CDD_RR_1_2;
static signed char CDD_RR_1_0;
static signed char CDD_RR_0_3;
static signed char CDD_RR_0_2;
static signed char CDD_RR_0_1;
static signed char CDD_WW_3_2;
static signed char CDD_WW_3_1;
static signed char CDD_WW_3_0;
static signed char CDD_WW_2_3;
static signed char CDD_WW_2_1;
static signed char CDD_WW_2_0;
static signed char CDD_WW_1_3;
static signed char CDD_WW_1_2;
static signed char CDD_WW_1_0;
static signed char CDD_WW_0_3;
static signed char CDD_WW_0_2;
static signed char CDD_WW_0_1;
static signed char CDD_RW_3_3;
static signed char CDD_RW_3_2;
static signed char CDD_RW_3_1;
static signed char CDD_RW_3_0;
static signed char CDD_RW_2_3;
static signed char CDD_RW_2_2;
static signed char CDD_RW_2_1;
static signed char CDD_RW_2_0;
static signed char CDD_RW_1_3;
static signed char CDD_RW_1_2;
static signed char CDD_RW_1_1;
static signed char CDD_RW_1_0;
static signed char CDD_RW_0_3;
static signed char CDD_RW_0_2;
static signed char CDD_RW_0_1;
static signed char CDD_RW_0_0;
static signed char CDD_WR_3_3;
static signed char CDD_WR_3_2;
static signed char CDD_WR_3_1;
static signed char CDD_WR_3_0;
static signed char CDD_WR_2_3;
static signed char CDD_WR_2_2;
static signed char CDD_WR_2_1;
static signed char CDD_WR_2_0;
static signed char CDD_WR_1_3;
static signed char CDD_WR_1_2;
static signed char CDD_WR_1_1;
static signed char CDD_WR_1_0;
static signed char CDD_WR_0_3;
static signed char CDD_WR_0_2;
static signed char CDD_WR_0_1;
static signed char CDD_WR_0_0;

static void _read_1d_message_block_outputs_(void)
{
    int rdata;

    //CHANNEL A
    rdata = dwc_ddrphy_apb_rd(0x54013);
    CDD_ChA_RR_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_RR_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54014);
    CDD_ChA_RW_1_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_RW_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54015);
    CDD_ChA_RW_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_RW_0_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54016);
    CDD_ChA_WR_1_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_WR_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54017);
    CDD_ChA_WR_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_WR_0_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54018);
    CDD_ChA_WW_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_WW_0_1 = ddr_get_data_field(rdata, 8, 8);

    //CHANNEL B
    rdata = dwc_ddrphy_apb_rd(0x5402c);
    CDD_ChB_RR_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402d);
    CDD_ChB_RR_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_RW_1_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402e);
    CDD_ChB_RW_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_RW_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402f);
    CDD_ChB_RW_0_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_WR_1_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54030);
    CDD_ChB_WR_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_WR_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54031);
    CDD_ChB_WR_0_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_WW_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54032);
    CDD_ChB_WW_0_1 = ddr_get_data_field(rdata, 0, 8);

    DDR_DBG("CDD_ChA_RR_1_0 = %d\n", CDD_ChA_RR_1_0);
    DDR_DBG("CDD_ChA_RR_0_1 = %d\n", CDD_ChA_RR_0_1);
    DDR_DBG("CDD_ChA_RW_1_1 = %d\n", CDD_ChA_RW_1_1);
    DDR_DBG("CDD_ChA_RW_1_0 = %d\n", CDD_ChA_RW_1_0);
    DDR_DBG("CDD_ChA_RW_0_1 = %d\n", CDD_ChA_RW_0_1);
    DDR_DBG("CDD_ChA_RW_0_0 = %d\n", CDD_ChA_RW_0_0);
    DDR_DBG("CDD_ChA_WR_1_1 = %d\n", CDD_ChA_WR_1_1);
    DDR_DBG("CDD_ChA_WR_1_0 = %d\n", CDD_ChA_WR_1_0);
    DDR_DBG("CDD_ChA_WR_0_1 = %d\n", CDD_ChA_WR_0_1);
    DDR_DBG("CDD_ChA_WR_0_0 = %d\n", CDD_ChA_WR_0_0);
    DDR_DBG("CDD_ChA_WW_1_0 = %d\n", CDD_ChA_WW_1_0);
    DDR_DBG("CDD_ChA_WW_0_1 = %d\n", CDD_ChA_WW_0_1);

    DDR_DBG("CDD_ChB_RR_1_0 = %d\n", CDD_ChB_RR_1_0);
    DDR_DBG("CDD_ChB_RR_0_1 = %d\n", CDD_ChB_RR_0_1);
    DDR_DBG("CDD_ChB_RW_1_1 = %d\n", CDD_ChB_RW_1_1);
    DDR_DBG("CDD_ChB_RW_1_0 = %d\n", CDD_ChB_RW_1_0);
    DDR_DBG("CDD_ChB_RW_0_1 = %d\n", CDD_ChB_RW_0_1);
    DDR_DBG("CDD_ChB_RW_0_0 = %d\n", CDD_ChB_RW_0_0);
    DDR_DBG("CDD_ChB_WR_1_1 = %d\n", CDD_ChB_WR_1_1);
    DDR_DBG("CDD_ChB_WR_1_0 = %d\n", CDD_ChB_WR_1_0);
    DDR_DBG("CDD_ChB_WR_0_1 = %d\n", CDD_ChB_WR_0_1);
    DDR_DBG("CDD_ChB_WR_0_0 = %d\n", CDD_ChB_WR_0_0);
    DDR_DBG("CDD_ChB_WW_1_0 = %d\n", CDD_ChB_WW_1_0);
    DDR_DBG("CDD_ChB_WW_0_1 = %d\n", CDD_ChB_WW_0_1);
}

static bool is_ddr4 = false;

static void _read_1d_message_block_outputs_ddr4_(void)
{
    int rdata;

    rdata = dwc_ddrphy_apb_rd(0x54012);
    CDD_RR_3_2 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54013);
    CDD_RR_3_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_3_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54014);
    CDD_RR_2_3 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_2_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54015);
    CDD_RR_2_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_1_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54016);
    CDD_RR_1_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54017);
    CDD_RR_0_3 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_0_2 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54018);
    CDD_RR_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_3_2 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54019);
    CDD_WW_3_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_3_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401a);
    CDD_WW_2_3 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_2_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401b);
    CDD_WW_2_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_1_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401c);
    CDD_WW_1_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401d);
    CDD_WW_0_3 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_0_2 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401e);
    CDD_WW_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_3_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401f);
    CDD_RW_3_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_3_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54020);
    CDD_RW_3_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_2_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54021);
    CDD_RW_2_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_2_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54022);
    CDD_RW_2_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_1_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54023);
    CDD_RW_1_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_1_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54024);
    CDD_RW_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_0_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54025);
    CDD_RW_0_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54026);
    CDD_RW_0_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_3_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54027);
    CDD_WR_3_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_3_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54028);
    CDD_WR_3_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_2_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54029);
    CDD_WR_2_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_2_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402a);
    CDD_WR_2_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_1_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402b);
    CDD_WR_1_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_1_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402c);
    CDD_WR_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_0_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402d);
    CDD_WR_0_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402e);
    CDD_WR_0_0 = ddr_get_data_field(rdata, 0, 8);

    DDR_DBG("CDD_RR_3_2= %d\n", CDD_RR_3_2);
    DDR_DBG("CDD_RR_3_1= %d\n", CDD_RR_3_1);
    DDR_DBG("CDD_RR_3_0= %d\n", CDD_RR_3_0);
    DDR_DBG("CDD_RR_2_3= %d\n", CDD_RR_2_3);
    DDR_DBG("CDD_RR_2_1= %d\n", CDD_RR_2_1);
    DDR_DBG("CDD_RR_2_0= %d\n", CDD_RR_2_0);
    DDR_DBG("CDD_RR_1_3= %d\n", CDD_RR_1_3);
    DDR_DBG("CDD_RR_1_2= %d\n", CDD_RR_1_2);
    DDR_DBG("CDD_RR_1_0= %d\n", CDD_RR_1_0);
    DDR_DBG("CDD_RR_0_3= %d\n", CDD_RR_0_3);
    DDR_DBG("CDD_RR_0_2= %d\n", CDD_RR_0_2);
    DDR_DBG("CDD_RR_0_1= %d\n", CDD_RR_0_1);
    DDR_DBG("CDD_WW_3_2= %d\n", CDD_WW_3_2);
    DDR_DBG("CDD_WW_3_1= %d\n", CDD_WW_3_1);
    DDR_DBG("CDD_WW_3_0= %d\n", CDD_WW_3_0);
    DDR_DBG("CDD_WW_2_3= %d\n", CDD_WW_2_3);
    DDR_DBG("CDD_WW_2_1= %d\n", CDD_WW_2_1);
    DDR_DBG("CDD_WW_2_0= %d\n", CDD_WW_2_0);
    DDR_DBG("CDD_WW_1_3= %d\n", CDD_WW_1_3);
    DDR_DBG("CDD_WW_1_2= %d\n", CDD_WW_1_2);
    DDR_DBG("CDD_WW_1_0= %d\n", CDD_WW_1_0);
    DDR_DBG("CDD_WW_0_3= %d\n", CDD_WW_0_3);
    DDR_DBG("CDD_WW_0_2= %d\n", CDD_WW_0_2);
    DDR_DBG("CDD_WW_0_1= %d\n", CDD_WW_0_1);
    DDR_DBG("CDD_RW_3_3= %d\n", CDD_RW_3_3);
    DDR_DBG("CDD_RW_3_2= %d\n", CDD_RW_3_2);
    DDR_DBG("CDD_RW_3_1= %d\n", CDD_RW_3_1);
    DDR_DBG("CDD_RW_3_0= %d\n", CDD_RW_3_0);
    DDR_DBG("CDD_RW_2_3= %d\n", CDD_RW_2_3);
    DDR_DBG("CDD_RW_2_2= %d\n", CDD_RW_2_2);
    DDR_DBG("CDD_RW_2_1= %d\n", CDD_RW_2_1);
    DDR_DBG("CDD_RW_2_0= %d\n", CDD_RW_2_0);
    DDR_DBG("CDD_RW_1_3= %d\n", CDD_RW_1_3);
    DDR_DBG("CDD_RW_1_2= %d\n", CDD_RW_1_2);
    DDR_DBG("CDD_RW_1_1= %d\n", CDD_RW_1_1);
    DDR_DBG("CDD_RW_1_0= %d\n", CDD_RW_1_0);
    DDR_DBG("CDD_RW_0_3= %d\n", CDD_RW_0_3);
    DDR_DBG("CDD_RW_0_2= %d\n", CDD_RW_0_2);
    DDR_DBG("CDD_RW_0_1= %d\n", CDD_RW_0_1);
    DDR_DBG("CDD_RW_0_0= %d\n", CDD_RW_0_0);
    DDR_DBG("CDD_WR_3_3= %d\n", CDD_WR_3_3);
    DDR_DBG("CDD_WR_3_2= %d\n", CDD_WR_3_2);
    DDR_DBG("CDD_WR_3_1= %d\n", CDD_WR_3_1);
    DDR_DBG("CDD_WR_3_0= %d\n", CDD_WR_3_0);
    DDR_DBG("CDD_WR_2_3= %d\n", CDD_WR_2_3);
    DDR_DBG("CDD_WR_2_2= %d\n", CDD_WR_2_2);
    DDR_DBG("CDD_WR_2_1= %d\n", CDD_WR_2_1);
    DDR_DBG("CDD_WR_2_0= %d\n", CDD_WR_2_0);
    DDR_DBG("CDD_WR_1_3= %d\n", CDD_WR_1_3);
    DDR_DBG("CDD_WR_1_2= %d\n", CDD_WR_1_2);
    DDR_DBG("CDD_WR_1_1= %d\n", CDD_WR_1_1);
    DDR_DBG("CDD_WR_1_0= %d\n", CDD_WR_1_0);
    DDR_DBG("CDD_WR_0_3= %d\n", CDD_WR_0_3);
    DDR_DBG("CDD_WR_0_2= %d\n", CDD_WR_0_2);
    DDR_DBG("CDD_WR_0_1= %d\n", CDD_WR_0_1);
    DDR_DBG("CDD_WR_0_0= %d\n", CDD_WR_0_0);
}

void _dwc_ddrphy_phyinit_userCustom_H_readMsgBlock_ (int Train2D)
{
    if (is_ddr4) {
        _read_1d_message_block_outputs_ddr4_();
    } else {
        _read_1d_message_block_outputs_();
    }
}

#define REG_AP_APB_DWC_DDR_UMCTL2_DBG1  (APB_DDRCTRL_BASE+UMCTL2_REGS_DBG1_OFF)
#define REG_AP_APB_DWC_DDR_UMCTL2_MSTR  (APB_DDRCTRL_BASE+UMCTL2_REGS_MSTR_OFF)
#define REG_AP_APB_DWC_DDR_UMCTL2_MSTR2  (APB_DDRCTRL_BASE+UMCTL2_REGS_MSTR2_OFF)
#define REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2 (APB_DDRCTRL_BASE+UMCTL2_REGS_DRAMTMG2_OFF)
#define DRAMTMG2_RD2WR_FIELD_OFFSET 8
#define DRAMTMG2_RD2WR_FIELD_SIZE 6
#define DRAMTMG2_WR2RD_FIELD_OFFSET 0
#define DRAMTMG2_WR2RD_FIELD_SIZE 6
#define REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL   (APB_DDRCTRL_BASE+UMCTL2_REGS_RANKCTL_OFF)
#define RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_OFFSET 26
#define RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_SIZE 1
#define RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_OFFSET 24
#define RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_SIZE 1
#define RANKCTL_DIFF_RANK_WR_GAP_FIELD_OFFSET 8
#define RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE 4
#define RANKCTL_DIFF_RANK_RD_GAP_FIELD_OFFSET 4
#define RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE 4
#define REG_AP_APB_DWC_DDR_UMCTL2_DFITMG1   (APB_DDRCTRL_BASE+UMCTL2_REGS_DFITMG1_OFF)
#define DFITMG1_DFI_T_WRDATA_DELAY_FIELD_OFFSET 16
#define DFITMG1_DFI_T_WRDATA_DELAY_FIELD_SIZE 5
#define DFITMG1_DFI_T_WRDATA_DELAY_FIELD_OFFSET 16
#define DFITMG1_DFI_T_WRDATA_DELAY_FIELD_SIZE 5
#define REG_AP_APB_DWC_DDR_UMCTL2_DFITMG1   (APB_DDRCTRL_BASE+UMCTL2_REGS_DFITMG1_OFF)
#define REG_AP_APB_DWC_DDR_UMCTL2_DBG1  (APB_DDRCTRL_BASE+UMCTL2_REGS_DBG1_OFF)

static void timing_reg_update_after_training_lpddr4(void)
{
    unsigned char CDD_ChA_RW;
    unsigned char CDD_ChB_RW;
    unsigned char CDD_ChA_WW;
    unsigned char CDD_ChB_WW;
    unsigned char CDD_ChA_RR;
    unsigned char CDD_ChB_RR;

    int rd2wr_old               ;
    int wr2rd_old               ;
    int diff_rank_wr_gap_old    ;
    int diff_rank_wr_gap_msb_old;
    int diff_rank_rd_gap_old    ;
    int diff_rank_rd_gap_msb_old;

    int rd2wr_new               ;
    int wr2rd_new               ;
    int diff_rank_wr_gap_new    ;
    int diff_rank_wr_gap_msb_new;
    int diff_rank_rd_gap_new    ;
    int diff_rank_rd_gap_msb_new;

    int TxDqsDlyTg0_lower_nibble;
    int TxDqsDlyTg0_upper_nibble;
    int TxDqsDlyTg1_lower_nibble;
    int TxDqsDlyTg1_upper_nibble;
    int TxDqsDly;
    int corse_delay;
#if defined(DDR_DEBUG)
    int fine_delay;
#endif
    int wrdata_delay_old;
    int wrdata_delay_new;

    CDD_ChA_RW = max_abs_4(CDD_ChA_RW_1_1, CDD_ChA_RW_1_0, CDD_ChA_RW_0_1, CDD_ChA_RW_0_0);
    CDD_ChB_RW = max_abs_4(CDD_ChB_RW_1_1, CDD_ChB_RW_1_0, CDD_ChB_RW_0_1, CDD_ChB_RW_0_0);

    CDD_ChA_WW = max_abs_2(CDD_ChA_WW_1_0, CDD_ChA_WW_0_1);
    CDD_ChB_WW = max_abs_2(CDD_ChB_WW_1_0, CDD_ChB_WW_0_1);

    CDD_ChA_RR = max_abs_2(CDD_ChA_RR_1_0, CDD_ChA_RR_0_1);
    CDD_ChB_RR = max_abs_2(CDD_ChB_RR_1_0, CDD_ChB_RR_0_1);

    DDR_DBG("CDD_ChA_RW = %d\n", CDD_ChA_RW);
    DDR_DBG("CDD_ChB_RW = %d\n", CDD_ChB_RW);
    DDR_DBG("CDD_ChA_WW = %d\n", CDD_ChA_WW);
    DDR_DBG("CDD_ChB_WW = %d\n", CDD_ChB_WW);
    DDR_DBG("CDD_ChA_RR = %d\n", CDD_ChA_RR);
    DDR_DBG("CDD_ChB_RR = %d\n", CDD_ChB_RR);

    rd2wr_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_RD2WR_FIELD_OFFSET, DRAMTMG2_RD2WR_FIELD_SIZE);
    wr2rd_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_WR2RD_FIELD_OFFSET, DRAMTMG2_WR2RD_FIELD_SIZE);
    diff_rank_wr_gap_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_WR_GAP_FIELD_OFFSET, RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE);
    diff_rank_wr_gap_msb_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_OFFSET, RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_SIZE);
    diff_rank_rd_gap_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_RD_GAP_FIELD_OFFSET, RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE);
    diff_rank_rd_gap_msb_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_OFFSET, RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_SIZE);

    DDR_DBG("rd2wr_old                = %d\n", rd2wr_old               );
    DDR_DBG("wr2rd_old                = %d\n", wr2rd_old               );
    DDR_DBG("diff_rank_wr_gap_old     = %d\n", diff_rank_wr_gap_old    );
    DDR_DBG("diff_rank_wr_gap_msb_old = %d\n", diff_rank_wr_gap_msb_old);
    DDR_DBG("diff_rank_rd_gap_old     = %d\n", diff_rank_rd_gap_old    );
    DDR_DBG("diff_rank_rd_gap_msb_old = %d\n", diff_rank_rd_gap_msb_old);

    rd2wr_new = 4 + rd2wr_old + (max2(CDD_ChA_RW, CDD_ChB_RW)+1)/2;
    wr2rd_new = 4 + wr2rd_old + (max2(CDD_ChA_WW, CDD_ChB_WW)+1)/2;

    DDR_DBG("11 rd2wr_new                = %d\n", rd2wr_new               );
    DDR_DBG("11 wr2rd_new                = %d\n", wr2rd_new               );

    diff_rank_wr_gap_new = (diff_rank_wr_gap_msb_old<<RANKCTL_DIFF_RANK_WR_GAP_FIELD_OFFSET) + diff_rank_wr_gap_old + (max2(CDD_ChA_WW, CDD_ChB_WW)+1)/2;
    DDR_DBG("11 diff_rank_wr_gap_new     = %d\n", diff_rank_wr_gap_new    );
    if(diff_rank_wr_gap_new <= 0xF) {
        diff_rank_wr_gap_new = diff_rank_wr_gap_new;
        diff_rank_wr_gap_msb_new = 0;
    }
    else {
        diff_rank_wr_gap_new     = ddr_get_data_field(diff_rank_wr_gap_new, 0, RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE);
        diff_rank_wr_gap_msb_new = ddr_get_data_field(diff_rank_wr_gap_new, RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE, RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_SIZE);
    }

    diff_rank_rd_gap_new = (diff_rank_rd_gap_msb_old<<RANKCTL_DIFF_RANK_RD_GAP_FIELD_OFFSET) + diff_rank_rd_gap_old + (max2(CDD_ChA_RR, CDD_ChB_RR)+1)/2;
    DDR_DBG("11 diff_rank_rd_gap_new     = %d\n", diff_rank_rd_gap_new);
    if(diff_rank_rd_gap_new <= 0xF) {
        diff_rank_rd_gap_new = diff_rank_rd_gap_new;
        diff_rank_rd_gap_msb_new = 0;
    }
    else {
        diff_rank_rd_gap_new     = ddr_get_data_field(diff_rank_rd_gap_new, 0, RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE);
        diff_rank_rd_gap_msb_new = ddr_get_data_field(diff_rank_rd_gap_new, RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE, RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_SIZE);
    }

    DDR_DBG("rd2wr_new                = %d\n", rd2wr_new               );
    DDR_DBG("wr2rd_new                = %d\n", wr2rd_new               );
    DDR_DBG("diff_rank_wr_gap_new     = %d\n", diff_rank_wr_gap_new    );
    DDR_DBG("diff_rank_wr_gap_msb_new = %d\n", diff_rank_wr_gap_msb_new);
    DDR_DBG("diff_rank_rd_gap_new     = %d\n", diff_rank_rd_gap_new    );
    DDR_DBG("diff_rank_rd_gap_msb_new = %d\n", diff_rank_rd_gap_msb_new);

    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000001);
    //ddrc_rd(REG_AP_APB_DWC_DDR_UMCTL2_MSTR);
    //ddrc_rd(REG_AP_APB_DWC_DDR_UMCTL2_MSTR2);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_RD2WR_FIELD_OFFSET, DRAMTMG2_RD2WR_FIELD_SIZE, rd2wr_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_WR2RD_FIELD_OFFSET, DRAMTMG2_WR2RD_FIELD_SIZE, wr2rd_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_WR_GAP_FIELD_OFFSET, RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE, diff_rank_wr_gap_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_OFFSET, RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_SIZE, diff_rank_wr_gap_msb_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_RD_GAP_FIELD_OFFSET, RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE, diff_rank_rd_gap_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_OFFSET, RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_SIZE, diff_rank_rd_gap_msb_new);

    //TxDqsDlyTg0-1
    TxDqsDlyTg0_lower_nibble = dwc_ddrphy_apb_rd(0x0100D0);
    TxDqsDlyTg0_upper_nibble = dwc_ddrphy_apb_rd(0x0101D0);
    TxDqsDlyTg1_lower_nibble = dwc_ddrphy_apb_rd(0x0100D1);
    TxDqsDlyTg1_upper_nibble = dwc_ddrphy_apb_rd(0x0101D1);

    TxDqsDly = max_abs_4int(TxDqsDlyTg0_lower_nibble, TxDqsDlyTg0_upper_nibble, TxDqsDlyTg1_lower_nibble, TxDqsDlyTg1_upper_nibble);

    corse_delay = ddr_get_data_field(TxDqsDly, 0, 5);
#if defined(DDR_DEBUG)
    fine_delay  = ddr_get_data_field(TxDqsDly, 6, 4);
#endif
    DDR_DBG("TxDqsDlyTg0_lower_nibble = %d\n", TxDqsDlyTg0_lower_nibble);
    DDR_DBG("TxDqsDlyTg0_upper_nibble = %d\n", TxDqsDlyTg0_upper_nibble);
    DDR_DBG("TxDqsDlyTg1_lower_nibble = %d\n", TxDqsDlyTg1_lower_nibble);
    DDR_DBG("TxDqsDlyTg1_upper_nibble = %d\n", TxDqsDlyTg1_upper_nibble);
    DDR_DBG("corse_delay = %d\n", corse_delay);
    DDR_DBG("fine_delay  = %d\n", fine_delay);

    wrdata_delay_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_DFITMG1, DFITMG1_DFI_T_WRDATA_DELAY_FIELD_OFFSET, DFITMG1_DFI_T_WRDATA_DELAY_FIELD_SIZE);
    wrdata_delay_new = wrdata_delay_old + ((corse_delay+1)/4); //1UI=1/2TCK
    DDR_DBG("wrdata_delay_old = %d\n", wrdata_delay_old);
    DDR_DBG("wrdata_delay_new = %d\n", wrdata_delay_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_DFITMG1, DFITMG1_DFI_T_WRDATA_DELAY_FIELD_OFFSET, DFITMG1_DFI_T_WRDATA_DELAY_FIELD_SIZE, wrdata_delay_new);
    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000000);
}

static void timing_reg_update_after_training_ddr4(void)
{
    unsigned char CDD_RW;
    int rd2wr_old ;
    int rd2wr_new ;

    CDD_RW = (CDD_RW_0_0>0)?CDD_RW_0_0:(-1*CDD_RW_0_0);

    DDR_DBG("CDD_RW = 0x%x\n", CDD_RW);

    rd2wr_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_RD2WR_FIELD_OFFSET, DRAMTMG2_RD2WR_FIELD_SIZE);

    DDR_DBG("rd2wr_old = %d\n", rd2wr_old);

    rd2wr_new = rd2wr_old + (CDD_RW+1)/2;

    DDR_DBG("rd2wr_new  = %d\n", rd2wr_new);

    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000001);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_RD2WR_FIELD_OFFSET, DRAMTMG2_RD2WR_FIELD_SIZE, rd2wr_new);
    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000000);
}

void _timing_reg_update_after_training_(void)
{
    if (is_ddr4) {
        timing_reg_update_after_training_ddr4();
    } else {
        timing_reg_update_after_training_lpddr4();
    }
}

void load_ddr_training_fw(uint32_t mem, uint32_t fw)
{
    uint16_t *p_fw = NULL;
    uint32_t sz = 0;
    uint32_t *to = NULL;

    DDR_DBG("going to load train FW(%s/%dd)...\n",
            DCCM == mem ? "DCCM" : "ICCM", (fw % 2) + 1);

    if ((fw == DDRPHY_FW_DDR4_1D) || (fw == DDRPHY_FW_DDR4_2D)) {
        is_ddr4 = true;
    }

    if (mem == DCCM) {
        p_fw =
#if LPDDR4X || CFG_DDR_TRAINING_4_ALL
                DDRPHY_FW_LPDDR4X_1D == fw ? (uint16_t *)ddrphy_fw_lpddr4x_dmem :
                DDRPHY_FW_LPDDR4X_2D == fw ? (uint16_t *)ddrphy_fw_lpddr4x_2d_dmem :
#endif
#if LPDDR4 || CFG_DDR_TRAINING_4_ALL
                DDRPHY_FW_LPDDR4_1D == fw ? (uint16_t *)ddrphy_fw_lpddr4_dmem :
                DDRPHY_FW_LPDDR4_2D == fw ? (uint16_t *)ddrphy_fw_lpddr4_2d_dmem :
#endif
#if DDR4 || CFG_DDR_TRAINING_4_ALL
                DDRPHY_FW_DDR4_1D == fw ? (uint16_t *)ddrphy_fw_ddr4_dmem :
                DDRPHY_FW_DDR4_2D == fw ? (uint16_t *)ddrphy_fw_ddr4_2d_dmem :
#endif
                NULL;
        sz =
#if LPDDR4X || CFG_DDR_TRAINING_4_ALL
            DDRPHY_FW_LPDDR4X_1D == fw ? (uint32_t)(ddrphy_fw_lpddr4x_dmem_end - ddrphy_fw_lpddr4x_dmem) :
            DDRPHY_FW_LPDDR4X_2D == fw ? (uint32_t)(ddrphy_fw_lpddr4x_2d_dmem_end - ddrphy_fw_lpddr4x_2d_dmem) :
#endif
#if LPDDR4 || CFG_DDR_TRAINING_4_ALL
            DDRPHY_FW_LPDDR4_1D == fw ? (uint32_t)(ddrphy_fw_lpddr4_dmem_end - ddrphy_fw_lpddr4_dmem) :
            DDRPHY_FW_LPDDR4_2D == fw ? (uint32_t)(ddrphy_fw_lpddr4_2d_dmem_end - ddrphy_fw_lpddr4_2d_dmem) :
#endif
#if DDR4 || CFG_DDR_TRAINING_4_ALL
            DDRPHY_FW_DDR4_1D == fw ? (uint32_t)(ddrphy_fw_ddr4_dmem_end - ddrphy_fw_ddr4_dmem) :
            DDRPHY_FW_DDR4_2D == fw ? (uint32_t)(ddrphy_fw_ddr4_2d_dmem_end - ddrphy_fw_ddr4_2d_dmem) :
#endif
            0ul;
        to = (uint32_t*)(APB_DDRPHY_BASE+0x54000*4);
    } else if (mem == ICCM) {
        p_fw =
#if LPDDR4X || CFG_DDR_TRAINING_4_ALL
                DDRPHY_FW_LPDDR4X_1D == fw ? (uint16_t *)ddrphy_fw_lpddr4x_imem :
                DDRPHY_FW_LPDDR4X_2D == fw ? (uint16_t *)ddrphy_fw_lpddr4x_2d_imem :
#endif
#if LPDDR4 || CFG_DDR_TRAINING_4_ALL
                DDRPHY_FW_LPDDR4_1D == fw ? (uint16_t *)ddrphy_fw_lpddr4_imem :
                DDRPHY_FW_LPDDR4_2D == fw ? (uint16_t *)ddrphy_fw_lpddr4_2d_imem :
#endif
#if DDR4 || CFG_DDR_TRAINING_4_ALL
                DDRPHY_FW_DDR4_1D == fw ? (uint16_t *)ddrphy_fw_ddr4_imem :
                DDRPHY_FW_DDR4_2D == fw ? (uint16_t *)ddrphy_fw_ddr4_2d_imem :
#endif
                NULL;
        sz =
#if LPDDR4X || CFG_DDR_TRAINING_4_ALL
            DDRPHY_FW_LPDDR4X_1D == fw ? (uint32_t)(ddrphy_fw_lpddr4x_imem_end - ddrphy_fw_lpddr4x_imem) :
            DDRPHY_FW_LPDDR4X_2D == fw ? (uint32_t)(ddrphy_fw_lpddr4x_2d_imem_end - ddrphy_fw_lpddr4x_2d_imem) :
#endif
#if LPDDR4 || CFG_DDR_TRAINING_4_ALL
            DDRPHY_FW_LPDDR4_1D == fw ? (uint32_t)(ddrphy_fw_lpddr4_imem_end - ddrphy_fw_lpddr4_imem) :
            DDRPHY_FW_LPDDR4_2D == fw ? (uint32_t)(ddrphy_fw_lpddr4_2d_imem_end - ddrphy_fw_lpddr4_2d_imem) :
#endif
#if DDR4 || CFG_DDR_TRAINING_4_ALL
            DDRPHY_FW_DDR4_1D == fw ? (uint32_t)(ddrphy_fw_ddr4_imem_end - ddrphy_fw_ddr4_imem) :
            DDRPHY_FW_DDR4_2D == fw ? (uint32_t)(ddrphy_fw_ddr4_2d_imem_end - ddrphy_fw_ddr4_2d_imem) :
#endif
            0ul;
        to = (uint32_t*)(APB_DDRPHY_BASE+0x50000*4);
    }

    if (sz && (NULL != p_fw)) {
        ddrphy_apb_memcpy(to, p_fw, sz);
    }
}

#ifdef __cplusplus
}
#endif
