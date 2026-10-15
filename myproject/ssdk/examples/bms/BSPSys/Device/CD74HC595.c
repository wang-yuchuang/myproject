/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CD74HC595.c
** @Founder      : ZQD808
** @Date         : 2025.11.11
** @Function     : CD74HC595移位寄存器通信模块
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
#include "BSPTypeDef.h"
#include "DEVGPIO.h"
#include "GroupInfo.h"

struct spi_controller *spi4_ctrl;
struct spi_device spi4_dev;
#if(BOARDX1_TYPE == 0)
struct spi_controller *spi8_ctrl;
struct spi_device spi8_dev;
#endif

static volatile bool s_spi4_busy = false;
#if(BOARDX1_TYPE == 0)
static volatile bool s_spi8_busy = false;
#endif

static uint8_t s_spi4_sendbuf[2] __attribute__((aligned(CONFIG_ARCH_CACHE_LINE)));
#if(BOARDX1_TYPE == 0)
static uint8_t s_spi8_sendbuf[2] __attribute__((aligned(CONFIG_ARCH_CACHE_LINE)));
#endif

static struct spi_transfer s_spi4_tx_trans;
static struct spi_msg s_spi4_tx_msg;
#if(BOARDX1_TYPE == 0)
static struct spi_transfer s_spi8_tx_trans;
static struct spi_msg s_spi8_tx_msg;
#endif

static void spi4_test_callback(int trans_stat, void *cb_param)
{
    if (trans_stat < 0) {
        ssdk_printf(SSDK_ERR, "SPI4 async transfer failed, stat=%d\r\n", trans_stat);
    }
    DEVGPIOSetPortState(ePOut_SPI4595_RCLK, ePSTATE_ON);
    DEVGPIOSetPortState(ePOut_SPI4595_RCLK, ePSTATE_OFF);
    s_spi4_busy = false;
}

#if(BOARDX1_TYPE == 0)
static void spi8_test_callback(int trans_stat, void *cb_param)
{
    if (trans_stat < 0) {
        ssdk_printf(SSDK_ERR, "SPI8 async transfer failed, stat=%d\r\n", trans_stat);
    }
    DEVGPIOSetPortState(ePOut_SPI8595_RCLK, ePSTATE_ON);
    DEVGPIOSetPortState(ePOut_SPI8595_RCLK, ePSTATE_OFF);
    s_spi8_busy = false;
}
#endif
/*=============================================================================================
** @Name      : int CD74HC595Init(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : CD74HC595初始化
** @The notes : 此函数需在系统SPI初始化后调用
**===========================================================================================*/
int CD74HC595Init(void)
{
    int ret;
    spi4_ctrl = spi_get_controller(4);
    if (spi4_ctrl == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: spi4 master get ctrl err!!!\r\n", __func__);
        return FALSE;
    }

    struct spi_device_config device4_conf = {
        .mode = (SPI_MODE_0),
        .freq_hz = 20000000,
        .bits_per_word = 8,
        //.chip_select = 0,
        //.cs_gpio =156,
        //.cs_gpio_enable = true,
    };

    ret = spi_init(&spi4_dev, spi4_ctrl, &device4_conf);

    if (ret < 0) {
        ssdk_printf(SSDK_WARNING, "%s: spi4 device init err!!!\r\n", __func__);
        return FALSE;
    }
    DEVGPIOSetPortState(ePOut_SPI4595_EN,ePSTATE_OFF);
    DEVGPIOSetPortState(ePOut_SPI4595_RCLK,ePSTATE_OFF);

    #if(BOARDX1_TYPE == 0)
    spi8_ctrl = spi_get_controller(8);
    if (spi8_ctrl == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: spi8 master get ctrl err!!!\r\n", __func__);
        return FALSE;
    }
    struct spi_device_config device8_conf = {
        .mode = (SPI_MODE_0),
        .freq_hz = 20000000,
        .bits_per_word = 8,
        //.chip_select = 0,
        //.cs_gpio =156,
        //.cs_gpio_enable = true,
    };

    ret = spi_init(&spi8_dev, spi8_ctrl, &device8_conf);

    if (ret < 0) {
        ssdk_printf(SSDK_WARNING, "%s: spi8 init err!!!\r\n", __func__);
        return FALSE;
    }
    DEVGPIOSetPortState(ePOut_SPI8595_EN,ePSTATE_OFF);
    DEVGPIOSetPortState(ePOut_SPI8595_RCLK,ePSTATE_OFF);
    #endif
    ssdk_printf(SSDK_WARNING, "%s: spi4 device init success!!!\r\n", __func__);
    return TRUE;
}

/*=============================================================================================
** @Name      :void CD74HC595SendDataAsync(e_595SPI_CHANNEL Channel, u16 data)
** @Input     : Channel:595通道 data:两个595控制IO数据（按bit）
** @Output    : 无
** @Function  : CD74HC595异步发送（发后即忘，回调中自动RCLK锁存）
** @The notes : 两个595级联，非阻塞
**===========================================================================================*/
void CD74HC595SendDataAsync(e_595SPI_CHANNEL Channel, u16 data)
{
    int ret = 0;
    uint8_t *sendbuf = NULL;
    struct spi_transfer *tx_trans = NULL;
    struct spi_msg *tx_msg = NULL;
    
    if((Channel != e_SPI4_595) && (Channel != e_SPI8_595))
        return;
    
    if(Channel == e_SPI4_595)
    {
        if(s_spi4_busy)
            return;
        
        sendbuf = s_spi4_sendbuf;
        sendbuf[0] = (u8)((data >> 8) & 0xff);
        sendbuf[1] = (u8)((data >> 0) & 0xff);
        
        tx_trans = &s_spi4_tx_trans;
        tx_msg = &s_spi4_tx_msg;
        
        memset(tx_trans, 0, sizeof(*tx_trans));
        tx_trans->tx_buf = (const void *)sendbuf;
        tx_trans->len = 2;

        memset(tx_msg, 0, sizeof(*tx_msg));
        tx_msg->trans_arr = tx_trans;
        tx_msg->trans_num = 1;
        tx_msg->cb = spi4_test_callback;
        tx_msg->cb_param = NULL;

        s_spi4_busy = true;
        ret = spi_transfer_async(&spi4_dev, tx_msg);
        if (ret < 0) {
            ssdk_printf(SSDK_ERR, "CD74HC595 spi4 async err, ret=%d\r\n", ret);
            s_spi4_busy = false;
        }
    }
    else if(Channel == e_SPI8_595)
    {
        #if(BOARDX1_TYPE == 0)
        if(s_spi8_busy)
            return;
        
        sendbuf = s_spi8_sendbuf;
        sendbuf[0] = (u8)((data >> 8) & 0xff);
        sendbuf[1] = (u8)((data >> 0) & 0xff);
        
        tx_trans = &s_spi8_tx_trans;
        tx_msg = &s_spi8_tx_msg;
        
        memset(tx_trans, 0, sizeof(*tx_trans));
        tx_trans->tx_buf = (const void *)sendbuf;
        tx_trans->len = 2;

        memset(tx_msg, 0, sizeof(*tx_msg));
        tx_msg->trans_arr = tx_trans;
        tx_msg->trans_num = 1;
        tx_msg->cb = spi8_test_callback;
        tx_msg->cb_param = NULL;

        s_spi8_busy = true;
        ret = spi_transfer_async(&spi8_dev, tx_msg);
        if (ret < 0) {
            ssdk_printf(SSDK_ERR, "CD74HC595 spi8 async err, ret=%d\r\n", ret);
            s_spi8_busy = false;
        }
        #endif
    }
}

