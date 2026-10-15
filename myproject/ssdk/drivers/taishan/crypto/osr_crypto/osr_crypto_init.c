/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <CLI_type.h>
#include <ral/utility.h>
#ifdef CONFIG_OSR_CRYPTO_DRIVER_MAILBOX
#include <driver/taishan/crypto/osr_crypto/mailbox/soc_seip_reg.h>
#endif

#define LOCAL_TRACE 1

#define APB_EFUSEC_BASE (0xf05f0000u)
#define EFUSEC_EHSM_CFG_OFFSET 0x1300
#define EFUSEC_SEIP_SECSTORAGE_SEL_OFFSET 0x12c9
#define APB_SCR_APB_BASE (0xf30f0000u)

#define APB_SEIP_BASE (0xf3100000u)
#define MODULE_RESET_CONTROL_OFF(n) (0x1304U + 8U*(n))
#define MODULE_RESET_CONTROL_BASE (0xF30D0000)
#define MODULE_RESET_CONTROL_RUN_MODE   (0x01U << 0U)
#define MODULE_AUTO_CLEAR_RESET_CONTROL_RUN_MODE   (0x01U << 6U)
#define MODULE_RESET_CONTROL_RSTA   (0x01U << 29U)

/*scr release seip rst scr_l16_reg
      ce_writel(0xF30F1200, 0x1);
      0xF30D0000+ 0x1304U + 8U*(10) = 0xF30D1354*/
#define MODULE_RESET_CONTROL_SEIP_BASE (0xF30D1354u)

#define RSTGEN_POLLs 0x10000

#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))
#define REG8(addr) ((volatile uint8_t *)(uintptr_t)(addr))

#define ce_writel(a, v) (*REG32(a) = (v))
#define ce_readl(a) (*REG32(a))
#define ce_writeb(a, v) (*REG8(a) = (v))
#define ce_readb(a) (*REG8(a))

#define SEIP_ERROR              (0x37)
#define SEIP_FW_VER_SUCCESS     (0xE6)

int osr_crypto_init()
{
    unsigned int  rdata = 0;
    uint32_t tms = 0;

    OSR_LTRACE("osr_crypto_init enter\n");

    /* if fuse config ssk4 from sec storage, set*/
    rdata = ce_readb(APB_EFUSEC_BASE + EFUSEC_SEIP_SECSTORAGE_SEL_OFFSET);
    if((rdata & 0x1) == 0){
        ce_writel(APB_SCR_APB_BASE + 0x1400, 0x2);
    }

    rdata = ce_readl(MODULE_RESET_CONTROL_SEIP_BASE);
    rdata |= MODULE_AUTO_CLEAR_RESET_CONTROL_RUN_MODE;

    ce_writel(MODULE_RESET_CONTROL_SEIP_BASE, rdata);

    rdata = ce_readl(MODULE_RESET_CONTROL_SEIP_BASE);
    OSR_LTRACE("read scr_l16_reg =0x%x\n",rdata);

    rdata |= MODULE_RESET_CONTROL_RUN_MODE;

    ce_writel(MODULE_RESET_CONTROL_SEIP_BASE, rdata);

    /*polling seip reset state*/
    while ((!(ce_readl(MODULE_RESET_CONTROL_SEIP_BASE) & MODULE_RESET_CONTROL_RSTA)) && (tms++ < RSTGEN_POLLs)) {

    }
    OSR_LTRACE("seip reset done\n");

    /*polling status seip boot_done*/
    do{
        rdata = ce_readl(APB_SEIP_BASE + 0x14);
    }while((rdata & 0x100) == 0);
    OSR_LTRACE("seip boot done\n");

#ifdef CONFIG_OSR_CRYPTO_DRIVER_MAILBOX
    /* if fuse config offset 16, seip_mode_sel = 10 CACC mode, 00 MAILBOX mode*/
    rdata = ce_readl(APB_EFUSEC_BASE + EFUSEC_EHSM_CFG_OFFSET);

    /* wait for seip ready for mailbox */
    if((rdata & 0x30000) == 0x0){

        while (rSOCMBOX_RSP_D0 == 0x0) {
        }

        rSOCMBOX_INT_STA = 0x02;

        if (rSOCMBOX_RSP_D0 == SEIP_FW_VER_SUCCESS) {
            OSR_LTRACE("mailbox init success\n");
        }
        else {
            OSR_LTRACE("mailbox init fail value=0x%x \n", rSOCMBOX_RSP_D0);
        }
    }else{
        OSR_LTRACE("cacc init success\n");
    }
#endif
    return 1;
}

static int osr_crypto_init_cmd(int argc, char* argv[])
{
    osr_crypto_init();
    return 0;
}

CLI_CMD("crypto_init", "\r\n crypto_init:\r\n start crypto init\r\n", osr_crypto_init_cmd);
