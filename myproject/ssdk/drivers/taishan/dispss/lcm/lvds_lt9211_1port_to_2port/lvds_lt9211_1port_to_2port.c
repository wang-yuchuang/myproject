/******************************************************************************
  * @project: LT9211
  * @file: lt9211.c
  * @author: zll
  * @company: LONTIUM COPYRIGHT and CONFIDENTIAL
  * @date:
	1��2019.04.10����2022.2.22 modify RXPLL to Adaptive BW tracking PLL. 8225 to 0x07 &8227 to 0x32
	2��V4.2 2022.5.7 add the 8223 to 0x02,disable the BTA function
 ******************************************************************************/

#include "lvds_lt9211_1port_to_2port.h"
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <driver/pinctrl.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <pinmux_cfg.h>

#define BACKLIGHT_GPIO GPIO_E5
#define LT9211_RESET_GPIO GPIO_E15

extern gpio_dev_t g_gpio_safety;
extern gpio_dev_t g_gpio_ap;

extern pinctrl_t g_pinctrl_safety;
extern pinctrl_t g_pinctrl_ap;

#define GPIO_INSTANCE(pin)                                                 \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap)

#define PINCTRL_INSTANCE(pin)                                               \
     (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap)

extern i2c_adap_dev_t *i2c8_adap_dev;
i2c_adap_dev_t **i2c_adap = &i2c8_adap_dev;
uint16_t i2c_slave_addr = 0x2D;

uint16_t hact, vact;
uint16_t hs, vs;
uint16_t hbp, vbp;
uint16_t htotal, vtotal;
uint16_t hfp, vfp;
uint8_t VideoFormat=0;
uint32_t lvds_clk_in = 0;

#define OFFSET 5

struct video_timing *pVideo_Format;

//hfp, hs, hbp, hact, htotal, vfp, vs, vbp, vact, vtotal, pixclk

struct video_timing video_1920x1080_60Hz   ={88, 44, 148,1920,  2200,  4,  5,  36, 1080, 1125, 148500};
struct video_timing video_1280x720_60Hz    ={110,40, 220,1280,  1650,  5,  5,  20, 720,   750,  74250};

struct video_timing video_1366x768_60Hz    ={26, 110,110,1366,  1592,  13, 6,  13, 768,   800,  81000};

void LT9211_Reset(void)
{
    gpio_dev_t *gpio_dev;
    pinctrl_t *pinctrl;

    gpio_dev = GPIO_INSTANCE(LT9211_RESET_GPIO);
    pinctrl = PINCTRL_INSTANCE(LT9211_RESET_GPIO);

    pinctrl_config(pinctrl, LT9211_RESET_GPIO, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT0);

    gpio_config(gpio_dev, LT9211_RESET_GPIO, GPIO_OUT);
    gpio_set(gpio_dev, LT9211_RESET_GPIO, 0);
    mdelay(100);
    gpio_set(gpio_dev, LT9211_RESET_GPIO, 1);
    mdelay(100);
}

int lt9211_i2c_write(i2c_adap_dev_t *adap, uint16_t addr,
                     const uint8_t *wbuf, int wlen)
{
    int ret;
    struct i2c_msg msgs[1] = {
        {
            .addr = addr,
            .buf = (uint8_t *)wbuf,
            .len = wlen,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 1);
    return ret;
}

int lt9211_i2c_write_read(i2c_adap_dev_t *adap, uint16_t addr,
                    const uint8_t *wbuf, int wlen,  uint8_t *rbuf, int rlen)
{
    int ret;
    struct i2c_msg msgs[2] = {
        {
            .addr = addr,
            .buf = (uint8_t *)wbuf,
            .len = wlen,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
        {
            .addr = addr,
            .buf = rbuf,
            .len = rlen,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_R,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 2);
    return ret;
}

bool HDMI_WriteI2C_Byte(uint8_t RegAddr, uint8_t data)
{
    int ret = 0;
    uint8_t buf[2];

    buf[0] = RegAddr;
    buf[1] = data;

    ret = lt9211_i2c_write(*i2c_adap, i2c_slave_addr, buf, 2);

    return ret;
}

uint8_t HDMI_ReadI2C_Byte(uint8_t RegAddr)
{
    uint8_t value = 0;

    lt9211_i2c_write_read(*i2c_adap, i2c_slave_addr, &RegAddr, 1, &value, 1);
    return value;
}

void LT9211_ChipID(void)
{
    HDMI_WriteI2C_Byte(0xff,0x81);//register bank
    DISPSS_LOG_INFO("LT9211 Chip ID:%x, %x, %x\n",HDMI_ReadI2C_Byte(0x00), HDMI_ReadI2C_Byte(0x01), HDMI_ReadI2C_Byte(0x02));
}

/** video chk soft rst **/
void lt9211_vid_chk_rst(void)
{
    HDMI_WriteI2C_Byte(0xff,0x81);
    HDMI_WriteI2C_Byte(0x10,0xbe);
    mdelay(10);
    HDMI_WriteI2C_Byte(0x10,0xfe);
}

/** lvds rx logic rst **/
void lt9211_lvdsrx_logic_rst(void)
{
    HDMI_WriteI2C_Byte(0xff,0x81);
    HDMI_WriteI2C_Byte(0x0c,0xeb);
    mdelay(10);
    HDMI_WriteI2C_Byte(0x0c,0xfb);
}

void LT9211_SystemInt(void)
{
    /* system clock init */
    HDMI_WriteI2C_Byte(0xff,0x82);
    HDMI_WriteI2C_Byte(0x01,0x18);

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x06,0x61);
    HDMI_WriteI2C_Byte(0x07,0xa8); //fm for sys_clk

    HDMI_WriteI2C_Byte(0xff,0x87); //��ʼ�� txpll �Ĵ����б�Ĭ��ֵ������
    HDMI_WriteI2C_Byte(0x14,0x08); //default value
    HDMI_WriteI2C_Byte(0x15,0x00); //default value
    HDMI_WriteI2C_Byte(0x18,0x0f);
    HDMI_WriteI2C_Byte(0x22,0x08); //default value
    HDMI_WriteI2C_Byte(0x23,0x00); //default value
    HDMI_WriteI2C_Byte(0x26,0x0f);
}

void LT9211_LvdsRxPhy(void)
{
    #ifdef INPUT_PORTA
    DISPSS_LOG_INFO("Port A PHY Config\n");
    HDMI_WriteI2C_Byte(0xff,0x82);
    HDMI_WriteI2C_Byte(0x02,0x8B);   //Port A LVDS mode enable
    HDMI_WriteI2C_Byte(0x05,0x21);   //port A CLK lane swap
    HDMI_WriteI2C_Byte(0x07,0x1f);   //port A clk enable
    HDMI_WriteI2C_Byte(0x04,0xa0);   //select port A clk as byteclk
    //HDMI_WriteI2C_Byte(0x09,0xFC); //port A P/N swap

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x33,0xe4);   //Port A Lane swap
    #endif

    #ifdef INPUT_PORTB
    DISPSS_LOG_INFO("Port B PHY Config\n");
    HDMI_WriteI2C_Byte(0xff,0x82);
    HDMI_WriteI2C_Byte(0x02,0x88);   //Port A/B LVDS mode enable
    HDMI_WriteI2C_Byte(0x05,0x21);   //port A CLK lane swap and rterm turn-off
    HDMI_WriteI2C_Byte(0x0d,0x21);   //port B CLK lane swap
    HDMI_WriteI2C_Byte(0x07,0x1f);   //port A clk enable  (ֻ��Portbʱ,porta��lane0 clkҪ��)
    HDMI_WriteI2C_Byte(0x0f,0x1f);   //port B clk enable
    //HDMI_WriteI2C_Byte(0x10,0x00);   //select port B clk as byteclk
    HDMI_WriteI2C_Byte(0x04,0xa1);   //reserve
    //HDMI_WriteI2C_Byte(0x11,0x01);   //port B P/N swap
    HDMI_WriteI2C_Byte(0x10,0xfc);

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x34,0xe4);   //Port B Lane swap

    HDMI_WriteI2C_Byte(0xff,0xd8);
    HDMI_WriteI2C_Byte(0x16,0x80);
    #endif

    HDMI_WriteI2C_Byte(0xff,0x81);
    HDMI_WriteI2C_Byte(0x20,0x7f);
    HDMI_WriteI2C_Byte(0x20,0xff);  //mlrx calib reset
}

void LT9211_LvdsRxDigital(void)
{
    HDMI_WriteI2C_Byte(0xff,0x85);
    HDMI_WriteI2C_Byte(0x88,0x10); //LVDS input, MIPI output

    HDMI_WriteI2C_Byte(0xff,0xd8);

    if (INPUT_PORT_NUM == 1) {            //1Port LVDS Input
        HDMI_WriteI2C_Byte(0x10,0x80);
        DISPSS_LOG_INFO("LVDS Port Num: 1\n");
    }
    else if (INPUT_PORT_NUM == 2) {       //2Port LVDS Input
        HDMI_WriteI2C_Byte(0x10,0x00);
        DISPSS_LOG_INFO("LVDS Port Num: 2\n");
    } else {
        DISPSS_LOG_INFO("Port Num Set Error\n");
    }

    lt9211_vid_chk_rst();              //video chk soft rst
    lt9211_lvdsrx_logic_rst();         //lvds rx logic rst

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x30,0x45);     //port AB input port sel

    if (LVDS_FORMAT == JEDIA_FORMAT) {
        HDMI_WriteI2C_Byte(0xff,0x85);
        HDMI_WriteI2C_Byte(0x59,0xd0);
        HDMI_WriteI2C_Byte(0xff,0xd8);
        HDMI_WriteI2C_Byte(0x11,0x40);
    }
}

void LT9211_ClockCheckDebug(void)
{
#ifdef _uart_debug_
    uint32_t fm_value;

    lvds_clk_in = 0;
    #ifdef INPUT_PORTA
    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x00,0x01);
    mdelay(50);
    fm_value = 0;
    fm_value = (HDMI_ReadI2C_Byte(0x08) &(0x0f));
    fm_value = (fm_value<<8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x09);
    fm_value = (fm_value<<8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x0a);
    DISPSS_LOG_INFO("Port A lvds clock: %d\n", fm_value);
    lvds_clk_in = fm_value;
    #endif

    #ifdef INPUT_PORTB
    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x00,0x02);
    mdelay(50);
    fm_value = 0;
    fm_value = (HDMI_ReadI2C_Byte(0x08) &(0x0f));
    fm_value = (fm_value<<8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x09);
    fm_value = (fm_value<<8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x0a);
    mdelay("Port B lvds clock: %d\n", fm_value);
    lvds_clk_in = fm_value;
    #endif

#endif
}

void LT9211_LvdsRxPll(void)
{
    uint8_t loopx = 0;

    HDMI_WriteI2C_Byte(0xff,0x82);
    HDMI_WriteI2C_Byte(0x25,0x07);
    HDMI_WriteI2C_Byte(0x27,0x32);

    if (INPUT_PORT_NUM == 1) {            //1Port LVDS Input
        HDMI_WriteI2C_Byte(0x24,0x24); //RXPLL_LVDSCLK_MUXSEL,PIXCLK_MUXSEL	0x2c.
        HDMI_WriteI2C_Byte(0x28,0x44);
    } else if(INPUT_PORT_NUM == 2) {       //2Port LVDS Input
        HDMI_WriteI2C_Byte(0x24,0x2c); //RXPLL_LVDSCLK_MUXSEL,PIXCLK_MUXSEL	0x2c.
        HDMI_WriteI2C_Byte(0x28,0x64); //0x64
    } else {
        DISPSS_LOG_INFO("LvdsRxPll: lvds port count error\n");
    }
    mdelay(10);
    HDMI_WriteI2C_Byte(0xff,0x81);
    HDMI_WriteI2C_Byte(0x20,0xdf);//rx pll reset
    HDMI_WriteI2C_Byte(0x20,0xff);
    mdelay(10);
    for (loopx = 0; loopx < 10; loopx++) { //Check Rx PLL lock
        HDMI_WriteI2C_Byte(0xff,0x87);
        if (HDMI_ReadI2C_Byte(0x12) & 0x80) {
//            if(HDMI_ReadI2C_Byte(0x11)& 0x80)
//            {
//                print("\r\nLT9211 rx cal done");
//            }
//            else
//            {
//                print("\r\nLT9211 rx cal undone!!");
//            }
            DISPSS_LOG_INFO("LT9211 rx pll lock\n");
            break;
        } else {
            DISPSS_LOG_INFO("LT9211 rx pll unlocked\n");
        }
    }
}

void LT9211_VideoCheck(void)
{
    uint8_t sync_polarity;

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x20,0x00);

    sync_polarity = HDMI_ReadI2C_Byte(0x70);
    vs = HDMI_ReadI2C_Byte(0x71);

    hs = HDMI_ReadI2C_Byte(0x72);
    hs = (hs << 8) + HDMI_ReadI2C_Byte(0x73);

    vbp = HDMI_ReadI2C_Byte(0x74);
    vfp = HDMI_ReadI2C_Byte(0x75);

    hbp = HDMI_ReadI2C_Byte(0x76);
    hbp = (hbp << 8) + HDMI_ReadI2C_Byte(0x77);

    hfp = HDMI_ReadI2C_Byte(0x78);
    hfp = (hfp << 8) + HDMI_ReadI2C_Byte(0x79);

    vtotal = HDMI_ReadI2C_Byte(0x7A);
    vtotal = (vtotal << 8) + HDMI_ReadI2C_Byte(0x7B);

    htotal = HDMI_ReadI2C_Byte(0x7C);
    htotal = (htotal << 8) + HDMI_ReadI2C_Byte(0x7D);

    vact = HDMI_ReadI2C_Byte(0x7E);
    vact = (vact << 8)+ HDMI_ReadI2C_Byte(0x7F);

    hact = HDMI_ReadI2C_Byte(0x80);
    hact = (hact << 8) + HDMI_ReadI2C_Byte(0x81);

    DISPSS_LOG_INFO("sync_polarity = %x\n", sync_polarity);
    if(!(sync_polarity & 0x01)) { //hsync
        HDMI_WriteI2C_Byte(0xff,0xd8);
        HDMI_WriteI2C_Byte(0x10, (HDMI_ReadI2C_Byte(0x10)| 0x10));
    }

    if (!(sync_polarity & 0x02)) { //vsync
        HDMI_WriteI2C_Byte(0xff,0xd8);
        HDMI_WriteI2C_Byte(0x10, (HDMI_ReadI2C_Byte(0x10)| 0x20));
    }

    DISPSS_LOG_INFO("hfp = %d, hs = %d, hbp = %d, hact = %d, htotal = %d\n", hfp, hs, hbp, hact, htotal);
    DISPSS_LOG_INFO("vfp = %d, vs = %d, vbp = %d, vact = %d, vtotal = %d\n", vfp, vs, vbp, vact, vtotal);

    if ((hact == video_1920x1080_60Hz.hact) && (vact == video_1920x1080_60Hz.vact)) {
        DISPSS_LOG_INFO("video_1920x1080_60Hz\n");
        pVideo_Format = &video_1920x1080_60Hz;
    } else if ((hact == video_1280x720_60Hz.hact) && (vact == video_1280x720_60Hz.vact)) {
        DISPSS_LOG_INFO("video_1280x720_60Hz\n");
        pVideo_Format = &video_1280x720_60Hz;
    } else if ((hact == video_1366x768_60Hz.hact) && (vact == video_1366x768_60Hz.vact)) {
        DISPSS_LOG_INFO("video_1366x768_60Hz\n");
        pVideo_Format = &video_1366x768_60Hz;
    } else {
        pVideo_Format = NULL;
        DISPSS_LOG_INFO("video_none\n");
    }
}

void LT9211_TxPhy(void)
{
    HDMI_WriteI2C_Byte(0xff,0x82);
    HDMI_WriteI2C_Byte(0x23,0x02);//disable the BTA
    HDMI_WriteI2C_Byte(0x62,0x00); //ttl output disable
    if (LVDS_PORTNUM == LVDS_2PORT) {
        HDMI_WriteI2C_Byte(0x3b,0x88);//dual-port lvds output disable
    } else {
        HDMI_WriteI2C_Byte(0x3b,0x08);//signal-port lvds output disable
    }
    HDMI_WriteI2C_Byte(0x3e,0x92);
    HDMI_WriteI2C_Byte(0x3f,0x48);
    HDMI_WriteI2C_Byte(0x40,0x31);
    HDMI_WriteI2C_Byte(0x43,0x80);
    HDMI_WriteI2C_Byte(0x44,0x00);
    HDMI_WriteI2C_Byte(0x45,0x00);
    HDMI_WriteI2C_Byte(0x49,0x00);
    HDMI_WriteI2C_Byte(0x4a,0x01);
    HDMI_WriteI2C_Byte(0x4e,0x00);
    HDMI_WriteI2C_Byte(0x4f,0x00);
    HDMI_WriteI2C_Byte(0x50,0x00);
    HDMI_WriteI2C_Byte(0x53,0x00);
    HDMI_WriteI2C_Byte(0x54,0x01);

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x46,0x10);
    #ifdef LVDS_2PORT_SWAP
    DISPSS_LOG_INFO("LVDS Output Port Swap!\n");
    HDMI_WriteI2C_Byte(0x46,0x40);
    #endif

    HDMI_WriteI2C_Byte(0xff,0x81);
    HDMI_WriteI2C_Byte(0x20,0x79); //TX PHY RESET & mlrx mltx calib reset
    HDMI_WriteI2C_Byte(0x20,0xff);
}

void LT9211_TxDigital(void)
{
    DISPSS_LOG_INFO("LT9211 LVDS_OUTPUT_MODE: \n");
    HDMI_WriteI2C_Byte(0xff,0x85); /* lvds tx controller */
    HDMI_WriteI2C_Byte(0x59,0x40);
    if (LVDS_DATAFORMAT == VESA) {
        DISPSS_LOG_INFO("Data Format: VESA\n");
        HDMI_WriteI2C_Byte(0x59, (HDMI_ReadI2C_Byte(0x59) & 0x7f));
    } else if (LVDS_DATAFORMAT == JEIDA) {
        DISPSS_LOG_INFO("Data Format: JEIDA\n");
        HDMI_WriteI2C_Byte(0x59, (HDMI_ReadI2C_Byte(0x59) | 0x80));
    } if (LVDS_COLORDEPTH == DEPTH_6BIT) {
        DISPSS_LOG_INFO("ColorDepth: 6Bit\n");
        HDMI_WriteI2C_Byte(0x59, (HDMI_ReadI2C_Byte(0x59) & 0xef));
    } else if (LVDS_COLORDEPTH == DEPTH_8BIT) {
        DISPSS_LOG_INFO("ColorDepth: 8Bit\n");
        HDMI_WriteI2C_Byte(0x59, (HDMI_ReadI2C_Byte(0x59) | 0x10));
    } if (LVDS_MODE == SYNC_MODE) {
        DISPSS_LOG_INFO("LVDS_MODE: Sync Mode\n");
        HDMI_WriteI2C_Byte(0x59, (HDMI_ReadI2C_Byte(0x59) & 0xdf));
    } else if (LVDS_MODE == DE_MODE) {
        DISPSS_LOG_INFO("LVDS_MODE: De Mode\n");
        HDMI_WriteI2C_Byte(0x59, (HDMI_ReadI2C_Byte(0x59) | 0x20));
    }

    HDMI_WriteI2C_Byte(0x5a,0xaa);
    HDMI_WriteI2C_Byte(0x5b,0xaa);
    if (LVDS_PORTNUM == LVDS_2PORT) {
        DISPSS_LOG_INFO("LVDS Output Port Num: 2Port\n");
        HDMI_WriteI2C_Byte(0x5c,0x03);	//lvdstx port sel 01:dual;00:single
    } else {
        DISPSS_LOG_INFO("LVDS Output Port Num: 1Port\n");
        HDMI_WriteI2C_Byte(0x5c,0x00);
    }
    HDMI_WriteI2C_Byte(0xa1,0x77);
    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x40,0x40); //tx_src_sel
    /*port src sel*/
    HDMI_WriteI2C_Byte(0x41,0x34);
    HDMI_WriteI2C_Byte(0x42,0x10);
    HDMI_WriteI2C_Byte(0x43,0x23); //pt0_tx_src_sel
    HDMI_WriteI2C_Byte(0x44,0x41);
    HDMI_WriteI2C_Byte(0x45,0x02); //pt1_tx_src_scl
}

void LT9211_Txpll(void)
{
    uint8_t loopx;

    HDMI_WriteI2C_Byte(0xff,0x82);
    HDMI_WriteI2C_Byte(0x36,0x01); //b7:txpll_pd
    if (LVDS_PORTNUM == LVDS_1PORT) {
        HDMI_WriteI2C_Byte(0x37,0x29);
    } else {
        HDMI_WriteI2C_Byte(0x37,0x2a);
    }
    HDMI_WriteI2C_Byte(0x38,0x06);
    HDMI_WriteI2C_Byte(0x39,0x30);
    HDMI_WriteI2C_Byte(0x3a,0x8e);

    HDMI_WriteI2C_Byte(0xFF,0x81);
    HDMI_WriteI2C_Byte(0x20,0xF7);// LVDS Txpll soft reset
    HDMI_WriteI2C_Byte(0x20,0xFF);

    HDMI_WriteI2C_Byte(0xff,0x87);
    HDMI_WriteI2C_Byte(0x37,0x14);
    HDMI_WriteI2C_Byte(0x13,0x00);
    HDMI_WriteI2C_Byte(0x13,0x80);
    mdelay(100);
    for (loopx = 0; loopx < 10; loopx++) { //Check Tx PLL cal
        HDMI_WriteI2C_Byte(0xff,0x87);
        if (HDMI_ReadI2C_Byte(0x1f) & 0x80) {
            if (HDMI_ReadI2C_Byte(0x20) & 0x80) {
                DISPSS_LOG_INFO("LT9211 tx pll lock\n");
            } else {
                DISPSS_LOG_INFO("LT9211 tx pll unlocked\n");
            }
            DISPSS_LOG_INFO("LT9211 tx pll cal done\n");
            break;
        } else {
            DISPSS_LOG_INFO("LT9211 tx pll unlocked\n");
        }
    }
}

void LT9211_LvdsClkDebug(void)
{
#ifdef _uart_debug_
    uint32_t fm_value;

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x00,0x12);
    mdelay(100);
    fm_value = 0;
    fm_value = (HDMI_ReadI2C_Byte(0x08) &(0x0f));
    fm_value = (fm_value<<8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x09);
    fm_value = (fm_value<<8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x0a);
    DISPSS_LOG_INFO("lvds output pixclk: %d\n", fm_value);
#endif
}

int lt9211_lvds_clkstb_check(void)
{
    uint8_t porta_clk_state = 0;
    uint8_t portb_clk_state = 0;

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x00,0x01);
    mdelay(300);
    porta_clk_state = (HDMI_ReadI2C_Byte(0x08) & (0x20));

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x00,0x02);
    mdelay(300);
    portb_clk_state = (HDMI_ReadI2C_Byte(0x08) & (0x20));

    if (INPUT_PORT_NUM == 1) {
        #ifdef INPUT_PORTA
        if (porta_clk_state) {
            return 1;
        } else {
            return 0;
        }
        #endif
        #ifdef INPUT_PORTB
        if (portb_clk_state) {
            return 1;
        } else {
            return 0;
        }
        #endif
    } else if (INPUT_PORT_NUM == 2) {
        if (porta_clk_state && portb_clk_state) {
            return 1;
        } else {
            return 0;
        }
    }
}

void lt9211_lvds_tx_logic_rst(void)
{
    HDMI_WriteI2C_Byte(0xff,0x81);
    HDMI_WriteI2C_Byte(0x0d,0xfb); //LVDS TX LOGIC RESET
    mdelay(10);
    HDMI_WriteI2C_Byte(0x0d,0xff); //LVDS TX LOGIC RESET  RELEASE
}



void lt9211_lvds_tx_en(void)
{
    HDMI_WriteI2C_Byte(0xff,0x82);

    if (LVDS_PORTNUM == LVDS_2PORT) {
        HDMI_WriteI2C_Byte(0x3b,0xb8);//dual-port lvds output Enable
    } else {
        HDMI_WriteI2C_Byte(0x3b,0x38);//signal-port lvds output Enable
    }
}

void lt9211_lvds_tx_disable(void)
{
    HDMI_WriteI2C_Byte(0xff,0x82);

    if (LVDS_PORTNUM == LVDS_2PORT) {
        HDMI_WriteI2C_Byte(0x3b,0x88);//dual-port lvds output disable
    } else {
        HDMI_WriteI2C_Byte(0x3b,0x08);//signal-port lvds output disable
    }
}

void LT9211_LVDS2LVDS_Config(void)
{
    DISPSS_LOG_INFO("*************LT9211 LVDS2LVDS Config**V4.1***********\n");
    LT9211_ChipID();
    LT9211_SystemInt();

  /********LVDS INPUT CONFIG********/
    LT9211_LvdsRxPhy();
    LT9211_LvdsRxDigital();
    LT9211_LvdsRxPll();
}

void Backlight_Set(void)
{
    gpio_dev_t *gpio_dev;
    pinctrl_t *pinctrl;

    gpio_dev = GPIO_INSTANCE(BACKLIGHT_GPIO);
    pinctrl = PINCTRL_INSTANCE(BACKLIGHT_GPIO);

    pinctrl_config(pinctrl, BACKLIGHT_GPIO, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT0);

    gpio_config(gpio_dev, BACKLIGHT_GPIO, GPIO_OUT);
    gpio_set(gpio_dev, BACKLIGHT_GPIO, 1);

}

void LT9211_LVDS2LVDS_MainLoop(void)
{
    static int flag_lvds_chg = 1;

    if (lt9211_lvds_clkstb_check()) {
        if (flag_lvds_chg) {
            DISPSS_LOG_INFO("lvds clk stable\n");

            LT9211_LvdsRxPll();
            lt9211_vid_chk_rst();           //video chk soft rst
            lt9211_lvdsrx_logic_rst();
            mdelay(10);
            LT9211_ClockCheckDebug();
            LT9211_VideoCheck();

            /********LVDS OUTPUT CONFIG********/
            LT9211_Txpll();
            LT9211_TxPhy();
            LT9211_TxDigital();

            lt9211_lvds_tx_logic_rst();    //LVDS TX LOGIC RESET
            lt9211_lvds_tx_en();           //LVDS TX output enable
            LT9211_LvdsClkDebug();

            flag_lvds_chg = 0;
        }
    } else {
        if (!flag_lvds_chg) {
            DISPSS_LOG_INFO("lvds clk not stable\n");
            lt9211_lvds_tx_disable();      //LVDS TX output disable
            flag_lvds_chg = 1;
        }
    }
    mdelay(1000);
    Backlight_Set();
}

void LT9211_LVDS2LVDS_Main(void)
{
    LT9211_Reset();
    LT9211_LVDS2LVDS_Config();
}
