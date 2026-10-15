/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SN74HC165.c
** @Founder      : ZQD808
** @Date         : 2025.11.11
** @Function     : 输入移位寄存器管理函数模块
** @Instructions : 根据PCB外围设备编写管理函数
**===========================================================================================*/
#include <arch.h>
#include <cmsis_os2.h>
#include <board.h>
#include <stdio.h>
#include <string.h>
#include "driver/spi.h"
#include "CLI_type.h"
#include "driver/gpio.h"
#include <driver/kunlun/spi/dw_spi_drv.h>
#include <regs_base.h>
#include <cmsis_os2.h>
#include "driver/pinctrl.h"
#include "driver/kunlun/pinctrl/sdrv_pinctrl.h"
#include "driver/gpio.h"
#include "driver/kunlun/gpio/sdrv_gpio.h"
#include "irq_num.h"
#include "param.h"
#include "CD74HC595.h"
#include "SN74HC165.h"
#include "BSPTypeDef.h"
#include "DEVGPIO.h"
#include "GroupInfo.h"

struct spi_controller *spi6_ctrl;
struct spi_device spi6_dev;

static volatile bool s_spi6_busy = false;
#if(BOARDX1_TYPE == 0)
static volatile bool s_spi3_busy = false;
#endif

static uint8_t s_spi6_readbuf[3] __attribute__((aligned(CONFIG_ARCH_CACHE_LINE)));
#if(BOARDX1_TYPE == 0)
static uint8_t s_spi3_readbuf[2] __attribute__((aligned(CONFIG_ARCH_CACHE_LINE)));
#endif

static struct spi_transfer s_spi6_rx_trans;
static struct spi_msg s_spi6_rx_msg;
#if(BOARDX1_TYPE == 0)
static struct spi_transfer s_spi3_rx_trans;
static struct spi_msg s_spi3_rx_msg;
#endif

static void spi6_test_callback(int trans_stat, void *cb_param)
{
    if (trans_stat < 0) {
        ssdk_printf(SSDK_ERR, "SPI6 async transfer failed, stat=%d\r\n", trans_stat);
    }
    SetMuiPortDiData(e_SPI6_165, 1, s_spi6_readbuf[0]);
    SetMuiPortDiData(e_SPI6_165, 2, s_spi6_readbuf[1]);
    SetMuiPortDiData(e_SPI6_165, 3, s_spi6_readbuf[2]);
    s_spi6_busy = false;
}

#if(BOARDX1_TYPE == 0)
static void spi3_test_callback(int trans_stat, void *cb_param)
{
    if (trans_stat < 0) {
        ssdk_printf(SSDK_ERR, "SPI3 async transfer failed, stat=%d\r\n", trans_stat);
    }
    SetMuiPortDiData(e_SPI3_165, 1, s_spi3_readbuf[0]);
    SetMuiPortDiData(e_SPI3_165, 2, s_spi3_readbuf[1]);
    s_spi3_busy = false;
}
#endif

#if(BOARDX1_TYPE == 0)
struct spi_controller *spi3_ctrl;
struct spi_device spi3_dev;
#endif
/*=============================================================================================
** @Name      : int SN74HC165Init(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : SN74HC165初始化
** @The notes : 此函数需在系统SPI初始化后调用
**===========================================================================================*/
int SN74HC165Init(void)
{
    int ret;
    spi6_ctrl = spi_get_controller(6);
    if (spi6_ctrl == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: spi6 master get ctrl err!!!\r\n", __func__);
        return FALSE;
    }

    struct spi_device_config device6_conf = {
        .mode = (SPI_MODE_0),
        .freq_hz = 20000000,
        .bits_per_word = 8,
        //.chip_select = 0,
        //.cs_gpio =156,
        //.cs_gpio_enable = true,
    };

    ret = spi_init(&spi6_dev, spi6_ctrl, &device6_conf);

    if (ret < 0) {
        ssdk_printf(SSDK_WARNING, "%s: spi6 device init err!!!\r\n", __func__);
        return FALSE;
    }
    DEVGPIOSetPortState(ePOut_SPI6165_CE,ePSTATE_OFF);
    DEVGPIOSetPortState(ePOut_SPI6165_PL,ePSTATE_OFF);

    #if(BOARDX1_TYPE == 0)
    spi3_ctrl = spi_get_controller(3);
    if (spi3_ctrl == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: spi3 master get ctrl err!!!\r\n", __func__);
        return FALSE;
    }

    struct spi_device_config device3_conf = {
        .mode = (SPI_MODE_0),
        .freq_hz = 20000000,
        .bits_per_word = 8,
        //.chip_select = 0,
        //.cs_gpio =156,
        //.cs_gpio_enable = true,
    };

    ret = spi_init(&spi3_dev, spi3_ctrl, &device3_conf);

    if (ret < 0) {
        ssdk_printf(SSDK_WARNING, "%s: spi3 device init err!!!\r\n", __func__);
        return FALSE;
    }
    DEVGPIOSetPortState(ePOut_SPI3165_CE,ePSTATE_OFF);
    DEVGPIOSetPortState(ePOut_SPI3165_PL,ePSTATE_OFF);
     
    #endif
    ssdk_printf(SSDK_WARNING, "%s: spi6 device init success!!!\r\n", __func__);
    return TRUE;
}

/*=============================================================================================
** @Name      :void SN74HC165GetDataAsync(void)
** @Input     : void
** @Output    : 无
** @Function  : SN74HC165异步读取（回调中自动解析数据）
** @The notes : 三个165级联，非阻塞
**===========================================================================================*/
void SN74HC165GetDataAsync(void)
{
    int ret = 0;
    
    if(s_spi6_busy)
        return;
    
    memset(s_spi6_readbuf, 0, sizeof(s_spi6_readbuf));
    
    DEVGPIOSetPortState(ePOut_SPI6165_PL, ePSTATE_OFF);
    DEVGPIOSetPortState(ePOut_SPI6165_PL, ePSTATE_ON);

    memset(&s_spi6_rx_trans, 0, sizeof(s_spi6_rx_trans));
    s_spi6_rx_trans.rx_buf = s_spi6_readbuf;
    s_spi6_rx_trans.len = 3;

    memset(&s_spi6_rx_msg, 0, sizeof(s_spi6_rx_msg));
    s_spi6_rx_msg.trans_arr = &s_spi6_rx_trans;
    s_spi6_rx_msg.trans_num = 1;
    s_spi6_rx_msg.cb = spi6_test_callback;
    s_spi6_rx_msg.cb_param = NULL;
    
    s_spi6_busy = true;
    ret = spi_transfer_async(&spi6_dev, &s_spi6_rx_msg);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "SN74HC165 spi6 async err, ret=%d\r\n", ret);
        s_spi6_busy = false;
    }

    #if(BOARDX1_TYPE == 0)
    if(s_spi3_busy)
        return;
    
    memset(s_spi3_readbuf, 0, sizeof(s_spi3_readbuf));
    DEVGPIOSetPortState(ePOut_SPI3165_PL, ePSTATE_OFF);
    DEVGPIOSetPortState(ePOut_SPI3165_PL, ePSTATE_ON);
    
    memset(&s_spi3_rx_trans, 0, sizeof(s_spi3_rx_trans));
    s_spi3_rx_trans.rx_buf = s_spi3_readbuf;
    s_spi3_rx_trans.len = 2;

    memset(&s_spi3_rx_msg, 0, sizeof(s_spi3_rx_msg));
    s_spi3_rx_msg.trans_arr = &s_spi3_rx_trans;
    s_spi3_rx_msg.trans_num = 1;
    s_spi3_rx_msg.cb = spi3_test_callback;
    s_spi3_rx_msg.cb_param = NULL;
    
    s_spi3_busy = true;
    ret = spi_transfer_async(&spi3_dev, &s_spi3_rx_msg);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "SN74HC165 spi3 async err, ret=%d\r\n", ret);
        s_spi3_busy = false;
    }
    #endif
}
