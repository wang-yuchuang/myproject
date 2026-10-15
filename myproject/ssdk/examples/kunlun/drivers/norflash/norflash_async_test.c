#include <stdio.h>
#include <arch.h>
#include <board.h>
#include <cmsis_os2.h>
#include <compiler.h>
#include <driver/clk.h>
#include <driver/spi_nor.h>

#include "CLI.h"
#include "irq_num.h"
#include "regs_base.h"
#include "driver/kunlun/spi_nor/cdns_ospi_drv.h"
#include "board.h"

#ifdef CONFIG_BOARD_X9SP
#undef DEVICE_INTR
#define DEVICE_INTR(dev) dev##_O_INTERRUPT_NUM
#endif

static struct spi_nor flash_fd;
static int step;
static uint8_t rx_buf[32] __ALIGNED(4) = {0};
static uint8_t tx_buf[32] __ALIGNED(4) = {
    0x44, 0x1c, 0x39, 0x05, 0xd3, 0x7a, 0x3c, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xaa, 0x9b, 0xf2, 0x7e, 0x01, 0x97, 0x60, 0x8c
};

int spi_nor_async_xfer(struct spi_nor *nor)
{
    enum flash_opt_result opt_result;

    opt_result = spi_nor_get_result(nor);
    if (step > 0 && step <= 3)
        printf("step%d: result = %d\n", step - 1, opt_result);

    if (opt_result == FLASH_OPT_PENDING || step > 3) {
        return -1;
    } else if (step == 3) {
        printf("check read data, data is: ");
        for(int i=0; i<sizeof(tx_buf); i++)
            printf("0x%x ", rx_buf[i]);
        printf("\n");

        printf("spinor async test end!\n");
        return 0;
    }

    switch (step) {
        case 0:
            printf("the original data is: ");
            for(int i=0; i<sizeof(tx_buf); i++)
                printf("0x%x ", tx_buf[i]);
            printf("\n");

            printf("step%d: test flash erase:\n", step);
            spi_nor_erase(&flash_fd, 0x1000, 0x1000);

            break;
        case 1:

            printf("step%d: test flash write:\n", step);
            spi_nor_write(&flash_fd, 0x1000, (const uint8_t *)tx_buf, sizeof(tx_buf));
           break;
        case 2:
            printf("step%d: test flash read:\n", step);
            spi_nor_read(&flash_fd, 0x1000, rx_buf, sizeof(tx_buf));
           break;
        default:
            break;
    }

    step++;
    return -1;
}

#if CONFIG_OSPI1_TEST
extern clk_node_t g_ckgen_ip_ospi1;
static struct cospi_pdata cospi1;
static struct spi_nor_host spi_nor_host1;

static int spinor_async_polling_test_ospi1(int argc, char *argv[])
{
    struct cospi_config host_config = {
        .id = 1,
        .apb_base = DEVICE_BASE(OSPI1),
        .ahb_base = OSPI1_BASE,
    };

    host_config.ref_clk = clk_get_rate(&g_ckgen_ip_ospi1);

    cospi_host_init(&spi_nor_host1, &cospi1, &host_config);

    struct spi_nor_config config = {
        .id = 1,
        .cs = 0,
        .baudrate = 50000000,
        .xfer_mode = SPI_NOR_XFER_POLLING_MODE,
        .async_mode = true,
    };
    if(spi_nor_init(&flash_fd, &spi_nor_host1, &config)){
        printf("spi_nor_init error\n");
        return 0;
    }

    printf("\nspinor async polling mode test begin:\n");

    step = 0;
    while (1) {
        spi_nor_main_function(&flash_fd);
        if (spi_nor_async_xfer(&flash_fd) == 0)
            break;
    }

    return 0;
}

static int spinor_async_interrupt_test_opsi1(int argc, char *argv[])
{
    struct cospi_config host_config = {
        .id = 1,
        .apb_base = DEVICE_BASE(OSPI1),
        .ahb_base = OSPI1_BASE,
        .irq = DEVICE_INTR(OSPI1),
    };

    host_config.ref_clk = clk_get_rate(&g_ckgen_ip_ospi1);

    cospi_host_init(&spi_nor_host1, &cospi1, &host_config);

    struct spi_nor_config config = {
        .id = 1,
        .cs = 0,
        .baudrate = 50000000,
        .xfer_mode = SPI_NOR_XFER_INTERRUPT_MODE,
        .async_mode = true,
    };
    if(spi_nor_init(&flash_fd, &spi_nor_host1, &config)){
        printf("spi_nor_init error\n");
        return 0;
    }

    printf("\nspinor async interrupt mode test begin:\n");

    step = 0;
    while (1) {
        spi_nor_main_function(&flash_fd);
        if (spi_nor_async_xfer(&flash_fd) == 0)
            break;
    }

    return 0;
}

/*
 * Register Internel CLI commands.
 */
CLI_CMD("spinor_async_polling_ospi1", "start spinor async polling uinttest", spinor_async_polling_test_ospi1);
CLI_CMD("spinor_async_interrupt_ospi1", "start spinor async interrupt uinttest", spinor_async_interrupt_test_opsi1);
#endif

#if CONFIG_OSPI2_TEST
extern clk_node_t g_ckgen_ip_ospi2;
static struct cospi_pdata cospi2;
static struct spi_nor_host spi_nor_host2;

static int spinor_async_polling_test_ospi2(int argc, char *argv[])
{
    struct cospi_config host_config = {
        .id = 2,
        .apb_base = DEVICE_BASE(OSPI2),
        .ahb_base = OSPI2_BASE,
    };

    host_config.ref_clk = clk_get_rate(&g_ckgen_ip_ospi2);

    cospi_host_init(&spi_nor_host2, &cospi2, &host_config);

    struct spi_nor_config config = {
        .id = 2,
        .cs = 0,
        .baudrate = 50000000,
        .xfer_mode = SPI_NOR_XFER_POLLING_MODE,
        .async_mode = true,
    };
    if(spi_nor_init(&flash_fd, &spi_nor_host2, &config)){
        printf("spi_nor_init error\n");
        return 0;
    }

    printf("\nspinor async polling mode test begin:\n");

    step = 0;
    while (1) {
        spi_nor_main_function(&flash_fd);
        if (spi_nor_async_xfer(&flash_fd) == 0)
            break;
    }

    return 0;
}

static int spinor_async_interrupt_test_ospi2(int argc, char *argv[])
{
    struct cospi_config host_config = {
        .id = 2,
        .apb_base = DEVICE_BASE(OSPI2),
        .ahb_base = OSPI2_BASE,
        .irq = DEVICE_INTR(OSPI2),
    };

    host_config.ref_clk = clk_get_rate(&g_ckgen_ip_ospi2);

    cospi_host_init(&spi_nor_host2, &cospi2, &host_config);

    struct spi_nor_config config = {
        .id = 2,
        .cs = 0,
        .baudrate = 50000000,
        .xfer_mode = SPI_NOR_XFER_INTERRUPT_MODE,
        .async_mode = true,
        .sw_rst = 1,
    };
    if(spi_nor_init(&flash_fd, &spi_nor_host2, &config)){
        printf("spi_nor_init error\n");
        return 0;
    }

    printf("\nspinor async interrupt mode test begin:\n");

    step = 0;
    while (1) {
        spi_nor_main_function(&flash_fd);
        if (spi_nor_async_xfer(&flash_fd) == 0)
            break;
    }

    return 0;
}

/*
 * Register Internel CLI commands.
 */
CLI_CMD("spinor_async_polling_ospi2", "start spinor async polling uinttest", spinor_async_polling_test_ospi2);
CLI_CMD("spinor_async_interrupt_ospi2", "start spinor async interrupt uinttest", spinor_async_interrupt_test_ospi2);
#endif
