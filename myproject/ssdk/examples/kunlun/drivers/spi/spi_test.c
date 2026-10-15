#include <arch.h>
#include <cmsis_os2.h>
#include <board.h>
#include <stdio.h>
#include <string.h>
#include "driver/spi.h"
#include "CLI_type.h"

static bool spi_cb_flag = false;

static void spi_test_callback(int trans_stat, void *cb_param)
{
    printf("spi cb stat=%d\n", trans_stat);
    spi_cb_flag = true;
}

static int spi_master_test(int argc, char *argv[])
{
    int ret;
    struct spi_controller *spi_ctrl;
    struct spi_device spi_dev;

    spi_ctrl = spi_get_controller(5);

    if (spi_ctrl == NULL) {
        printf("spi master get ctrl err\n");
        return CLI_FALSE;
    }

    struct spi_device_config device_conf = {
        .mode = (SPI_MODE_0),
        //.mode = (SPI_MODE_0) | SPI_LOOP,
        .freq_hz = 1000000,
        .bits_per_word = 8,
        .chip_select = 0,
        //.cs_gpio =156,
        //.cs_gpio_enable = true,
    };

    ret = spi_init(&spi_dev, spi_ctrl, &device_conf);

    if (ret < 0) {
        printf("spi device init err ret=%d\n", ret);
        return CLI_FALSE;
    }

    printf("---spi sync start---\n");

    uint8_t s_tbuf[48] = {0};
    uint8_t s_rbuf[48] = {0};

    for (int i = 0; i < 48; i++)
        s_tbuf[i] = i + 1;

    //case 1 spi write
    spi_write(&spi_dev, s_tbuf, 48);

    //case 2 spi read
    memset(s_rbuf, 0x55, 48);
    spi_read(&spi_dev, s_rbuf, 48);

    for (int i = 0; i < 48; i++)
        printf("read rbuf[%d]=%x\n", i, s_rbuf[i]);

    //case 3 spi write then read
    memset(s_rbuf, 0x55, 48);
    spi_write_then_read(&spi_dev, s_tbuf, s_rbuf, 48, 48);

    for (int i = 0; i < 48; i++)
        printf("write_then_read rbuf[%d]=%x\n", i, s_rbuf[i]);

    //case 4 spi parallel write and read
    memset(s_rbuf, 0x55, 48);
    spi_parallel_rw(&spi_dev, s_tbuf, s_rbuf, 48);

    for (int i = 0; i < 48; i++)
        printf("parallel_rw rbuf[%d]=%x\n", i, s_rbuf[i]);

    printf("---spi sync end---\n");

    printf("---spi async start---\n");

    uint8_t as_tbuf[50] = {0};
    uint8_t as_rbuf[50] = {0};

    for (int i = 0; i < 50; i++)
        as_tbuf[i] = i + 1;

    memset(as_rbuf, 0x55, 50);

#if 0
    //case 1 spi write
    struct spi_transfer tx_trans = {
        .tx_buf = (const void *)as_tbuf,
        .len = 50
    };

    struct spi_msg tx_msg = {
        .trans_arr = &tx_trans,
        .trans_num = 1,
        .cb = spi_test_callback,
        .cb_param = NULL,
    };

    ret = spi_transfer_async(&spi_dev, &tx_msg);

    if (ret < 0)
        printf("spi async err write ret=%d\n", ret);
#endif

#if 0
    //case 2 spi read
    struct spi_transfer rx_trans = {
        .rx_buf = as_rbuf,
        .len = 50
    };

    struct spi_msg rx_msg = {
        .trans_arr = &rx_trans,
        .trans_num = 1,
        .cb = spi_test_callback,
        .cb_param = NULL,
    };

    ret = spi_transfer_async(&spi_dev, &rx_msg);

    if (ret < 0)
        printf("spi async err read ret=%d\n", ret);
#endif

#if 1
    //case 3 spi write then read
    struct spi_transfer tx_then_rx_trans[2] = {
        {
            .tx_buf = (const void *)as_tbuf,
            .len = 50
        },
        {
            .rx_buf = as_rbuf,
            .len = 50
        },
    };

    struct spi_msg tx_then_rx_msg = {
        .trans_arr = tx_then_rx_trans,
        .trans_num = 2,
        .cb = spi_test_callback,
        .cb_param = NULL,
    };

    ret = spi_transfer_async(&spi_dev, &tx_then_rx_msg);

    if (ret < 0)
        printf("spi async err write then read ret=%d\n", ret);
#endif

#if 0
    //case 4 spi parallel write and read
    struct spi_transfer tx_and_rx_trans = {
        .tx_buf = as_tbuf,
        .rx_buf = as_rbuf,
        .len = 50
    };

    struct spi_msg tx_and_rx_msg = {
        .trans_arr = &tx_and_rx_trans,
        .trans_num = 1,
        .cb = spi_test_callback,
        .cb_param = NULL,
    };

    ret = spi_transfer_async(&spi_dev, &tx_and_rx_msg);

    if (ret < 0)
        printf("spi async err parallel write and read ret = %d\n", ret);
#endif

    while (!spi_cb_flag)
        osDelay(10);

    spi_cb_flag = false;

    for (int i = 0; i < 50; i++)
        printf("as_rbuf[%d]=%x\n", i, as_rbuf[i]);

    printf("---spi async end---\n");
    return CLI_FALSE;
}
CLI_CMD("spim_test", "spi master unittest", spi_master_test);

static void spi_slv_thread(void *arg)
{
    int ret;
    struct spi_controller *spi_ctrl = spi_get_controller(6);

    if (spi_ctrl == NULL) {
        printf("spi slv get ctrl err\n");
        return ;
    }

    uint8_t txbuf[32];
    uint8_t rxbuf[32];

    for (int i = 0; i < 32; i++) {
        txbuf[i] = i + 1;
        rxbuf[i] = 0x55;
    }

    while (1) {
        memset(rxbuf, 0x55, 32);

        //case 1 slave send and receive
        ret = spi_slave_transfer(spi_ctrl, txbuf, rxbuf, 32);

        //case 2 slave send only
        //ret = spi_slave_transfer(spi_ctrl, txbuf, NULL, 32);
        //case 3 slave receive only
        //ret = spi_slave_transfer(spi_ctrl, NULL, rxbuf, 32);
        if (ret < 0) {
            printf("spi slave transfer err %d\n", ret);
            return ;
        }

        for (int i = 0; i < 32; i++)
            printf("rxbuf[%d]=%x\n", i, rxbuf[i]);
    }
}

static int spi_slv_test(int argc, char *argv[])
{
	osThreadNew(spi_slv_thread, NULL, NULL);
    return CLI_FALSE;
}
CLI_CMD("spis_test", "spi slave unittest", spi_slv_test);

