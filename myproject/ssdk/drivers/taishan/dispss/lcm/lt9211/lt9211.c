/******************************************************************************
  * @project: LT9211
  * @author: zll
  * @company: LONTIUM COPYRIGHT and CONFIDENTIAL
  * @date: 2019.04.10
******************************************************************************/

#include "lt9211.h"
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <driver/pinctrl.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <pinmux_cfg.h>

#define BACKLIGHT_GPIO GPIO_E5
#define LT9211_RESET_GPIO GPIO_E21
#define LCD_RESET_GPIO GPIO_E22

extern gpio_dev_t g_gpio_safety;
extern gpio_dev_t g_gpio_ap;

extern pinctrl_t g_pinctrl_safety;
extern pinctrl_t g_pinctrl_ap;

#define GPIO_INSTANCE(pin)                                                 \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap)

#define PINCTRL_INSTANCE(pin)                                               \
     (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap)

uint16_t hact, vact;
uint16_t hs, vs;
uint16_t hbp, vbp;
uint16_t htotal, vtotal;
uint16_t hfp, vfp;
uint8_t VideoFormat=0;
uint32_t lvds_clk_in = 0;

enum VideoFormat Video_Format;
//hfp, hs, hbp,hact,htotal,vfp, vs, vbp, vact,vtotal,

struct video_timing video_480x1920_60Hz = {30, 30, 30, 480, 570, 6, 6, 6, 1920, 1938, 67000};

extern i2c_adap_dev_t *i2c8_adap_dev;
i2c_adap_dev_t **i2c_adap = &i2c8_adap_dev;
uint16_t i2c_slave_addr = 0x2D;

uint8_t Sleep_Out[] = {0x05, 0x11, 0x00};
uint8_t Sleep_IN[] = {0x05, 0x10, 0x00};
uint8_t Display_On[] = {0x05, 0x29, 0x00};


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

void LCD_Reset(void)
{
    gpio_dev_t *gpio_dev;
    pinctrl_t *pinctrl;

    gpio_dev = GPIO_INSTANCE(LCD_RESET_GPIO);
    pinctrl = PINCTRL_INSTANCE(LCD_RESET_GPIO);

    pinctrl_config(pinctrl, LCD_RESET_GPIO, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT0);

    gpio_config(gpio_dev, LCD_RESET_GPIO, GPIO_OUT);
    gpio_set(gpio_dev, LCD_RESET_GPIO, 0);
    mdelay(50);
    gpio_set(gpio_dev, LCD_RESET_GPIO, 1);
    mdelay(20);

}

void DcsPktWrite(uint8_t DCS_DI, uint8_t Len, uint8_t* Ptr )
{
    uint8_t i = 0;

    HDMI_WriteI2C_Byte(0xff, 0xd4);

    if (Len == 2) {
        HDMI_WriteI2C_Byte(0x01, 0x0c);
        HDMI_WriteI2C_Byte(0x02, 0x04);
        HDMI_WriteI2C_Byte(0x03, DCS_DI);
        HDMI_WriteI2C_Byte(0x03, *Ptr);
        HDMI_WriteI2C_Byte(0x03, *(Ptr + 1));
    }
    else {
        HDMI_WriteI2C_Byte(0x01, 0x0E);
        HDMI_WriteI2C_Byte(0x02, Len + 6);
        HDMI_WriteI2C_Byte(0x03, DCS_DI);
        HDMI_WriteI2C_Byte(0x03, Len);
        HDMI_WriteI2C_Byte(0x03, 0x00);

        for (i = 0; i < Len; i++) {
            HDMI_WriteI2C_Byte(0x03, *Ptr);
            Ptr++;
        }
    }

    mdelay(1);
    HDMI_WriteI2C_Byte(0x01, 0x00);
}


void InitPanel( void )
{
    HDMI_WriteI2C_Byte(0xff,0x81);
    HDMI_WriteI2C_Byte(0x0e,0xef);
    HDMI_WriteI2C_Byte(0x0e,0xff);
    HDMI_WriteI2C_Byte(0x0b,0xfe);

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x47,0x01);
    HDMI_WriteI2C_Byte(0x48,0x01);

    HDMI_WriteI2C_Byte(0xff,0xd4);
    HDMI_WriteI2C_Byte(0x20,0x2f);
    HDMI_WriteI2C_Byte(0x21,0x10);

    LCD_Reset();

    DcsPktWrite(Sleep_Out[0], (sizeof(Sleep_Out) - 1), &Sleep_Out[1]);
    mdelay(300);

    DcsPktWrite(Display_On[0], (sizeof(Display_On) - 1), &Display_On[1]);
    mdelay(300);

    DISPSS_LOG_INFO("Finish initial panel\n");
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
    HDMI_WriteI2C_Byte(0x88,0x10);      //LVDS input, MIPI output

    HDMI_WriteI2C_Byte(0xff,0xd8);

    if (INPUT_PORT_NUM == 1) {            //1Port LVDS Input
        HDMI_WriteI2C_Byte(0x10,0x80);
        DISPSS_LOG_INFO("LVDS Port Num: 1\n");
    } else if (INPUT_PORT_NUM == 2) {       //2Port LVDS Input
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

void LT9211_ClockCheckDebug(void)
{
    uint32_t fm_value;

    lvds_clk_in = 0;
    #ifdef INPUT_PORTA
    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x00,0x01);
    mdelay(50);
    fm_value = 0;
    fm_value = (HDMI_ReadI2C_Byte(0x08) & (0x0f));
    fm_value = (fm_value << 8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x09);
    fm_value = (fm_value << 8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x0a);
    DISPSS_LOG_INFO("Port A lvds clock: %d\n", fm_value);
    lvds_clk_in = fm_value;
    #endif

    #ifdef INPUT_PORTB
    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x00,0x02);
    mdelay(50);
    fm_value = 0;
    fm_value = (HDMI_ReadI2C_Byte(0x08) & (0x0f));
    fm_value = (fm_value << 8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x09);
    fm_value = (fm_value << 8) ;
    fm_value = fm_value + HDMI_ReadI2C_Byte(0x0a);
    DISPSS_LOG_INFO("Port B lvds clock: %d\n", fm_value);
    lvds_clk_in = fm_value;
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
        HDMI_WriteI2C_Byte(0x28,0x44); //0x64
    } else if (INPUT_PORT_NUM == 2) {       //2Port LVDS Input
        HDMI_WriteI2C_Byte(0x24,0x2c); //RXPLL_LVDSCLK_MUXSEL,PIXCLK_MUXSEL	0x2c.
        HDMI_WriteI2C_Byte(0x28,0x64); //0x64
    } else {
        DISPSS_LOG_INFO("LvdsRxPll: lvds port count error\n");
    }
    mdelay(10);
//    HDMI_WriteI2C_Byte(0xff,0x87);
 //   HDMI_WriteI2C_Byte(0x05,0x00);
   // HDMI_WriteI2C_Byte(0x05,0x80);
      HDMI_WriteI2C_Byte(0xff,0x81);
      HDMI_WriteI2C_Byte(0x20,0xdf);
      HDMI_WriteI2C_Byte(0x20,0xff);
    mdelay(100);
    for (loopx = 0; loopx < 10; loopx++) { //Check Rx PLL cal
        HDMI_WriteI2C_Byte(0xff,0x87);

        if (HDMI_ReadI2C_Byte(0x12) & 0x80) {
            if (HDMI_ReadI2C_Byte(0x11) & 0x80) {
                DISPSS_LOG_INFO("LT9211 rx cal done\n");
            } else {
                DISPSS_LOG_INFO("LT9211 rx cal undone!!\n");
            }

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
	if (!(sync_polarity & 0x01)) { //hsync
        HDMI_WriteI2C_Byte(0xff,0xd8);
        HDMI_WriteI2C_Byte(0x10, (HDMI_ReadI2C_Byte(0x10) | 0x10));
	}
	if (!(sync_polarity & 0x02)) { //vsync
        HDMI_WriteI2C_Byte(0xff,0xd8);
        HDMI_WriteI2C_Byte(0x10, (HDMI_ReadI2C_Byte(0x10) | 0x20));
	}

    DISPSS_LOG_INFO("hfp = %d, hs = %d, hbp = %d, hact = %d, htotal = %d\n", hfp, hs, hbp, hact, htotal);

    DISPSS_LOG_INFO("vfp = %d, vs = %d, vbp = %d, vact = %d, vtotal = %d\n", vfp, vs, vbp, vact, vtotal);
}

void LT9211_MipiTxpll(void)
{
    uint8_t loopx;

    HDMI_WriteI2C_Byte(0xff,0x82);
    HDMI_WriteI2C_Byte(0x36,0x03); //b7:txpll_pd
    HDMI_WriteI2C_Byte(0x37,0x28);
    HDMI_WriteI2C_Byte(0x38,0x04);
    HDMI_WriteI2C_Byte(0x3a,0x94);
// HDMI_WriteI2C_Byte(0x3b,0x44);

    HDMI_WriteI2C_Byte(0xff,0x87);
    HDMI_WriteI2C_Byte(0x13,0x00);
    HDMI_WriteI2C_Byte(0x13,0x80);
    mdelay(100);
    for (loopx = 0; loopx < 10; loopx++) { //Check Tx PLL cal done
        HDMI_WriteI2C_Byte(0xff,0x87);
        if (HDMI_ReadI2C_Byte(0x1f)& 0x80) {
            if (HDMI_ReadI2C_Byte(0x20)& 0x80) {
                DISPSS_LOG_INFO("LT9211 tx pll lock\n");
            } else {
                DISPSS_LOG_INFO("LT9211 tx pll unlocked\n");
            }
            DISPSS_LOG_INFO("LT9211 tx pll cal done\n");
            break;
        } else {
            DISPSS_LOG_INFO("LT9211 tx pll cal undone\n");
        }
    }
}


void LT9211_MipiTxPhy(void)
{
    DISPSS_LOG_INFO("LT9211 tx phy\n");
    HDMI_WriteI2C_Byte(0xff,0x82);
    HDMI_WriteI2C_Byte(0x62,0x00); //ttl output disable
    HDMI_WriteI2C_Byte(0x3b,0x32); //mipi en
//    HDMI_WriteI2C_Byte(0x48,0x5f); //Port A Lane P/N Swap
//    HDMI_WriteI2C_Byte(0x49,0x92);
//    HDMI_WriteI2C_Byte(0x52,0x5f); //Port B Lane P/N Swap
//    HDMI_WriteI2C_Byte(0x53,0x92);

    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x40,0x80); //tx_src_sel
    /*port src sel*/
    HDMI_WriteI2C_Byte(0x41,0x01);
    HDMI_WriteI2C_Byte(0x42,0x23);
    HDMI_WriteI2C_Byte(0x43,0x40); //Port A MIPI Lane Swap
    HDMI_WriteI2C_Byte(0x44,0x12);
    HDMI_WriteI2C_Byte(0x45,0x34); //Port B MIPI Lane Swap
}

void LT9211_MipiTxDigital(void)
{
    DISPSS_LOG_INFO("LT9211 tx digital\n");
    HDMI_WriteI2C_Byte(0xff,0xd4);
    HDMI_WriteI2C_Byte(0x1c,0x30);  //hs_rqst_pre
    HDMI_WriteI2C_Byte(0x1d,0x0a);  //lpx
    HDMI_WriteI2C_Byte(0x1e,0x06);  //prpr
    HDMI_WriteI2C_Byte(0x1f,0x0a);  //trail
    HDMI_WriteI2C_Byte(0x21,0x00);  //[5]byte_swap,[0]burst_clk

    HDMI_WriteI2C_Byte(0xff,0xd4);
    HDMI_WriteI2C_Byte(0x16,0x55);
    HDMI_WriteI2C_Byte(0x10,0x01);
    HDMI_WriteI2C_Byte(0x11,0x50); //read byteclk ??,???
    HDMI_WriteI2C_Byte(0x13,0x0f);	//bit[5:4]:lane num, bit[2]:bllp,bit[1:0]:vid_mode
    HDMI_WriteI2C_Byte(0x14,0x20); //bit[5:4]:data typ,bit[2:0]:fmt sel 000:rgb888
    HDMI_WriteI2C_Byte(0x21,0x03);
}

void LT9211_SetTxTiming(void)
{
    HDMI_WriteI2C_Byte(0xff,0xd4);
    HDMI_WriteI2C_Byte(0x04,0x08); //hs[7:0] not care
    HDMI_WriteI2C_Byte(0x05,0x08); //hbp[7:0] not care
    HDMI_WriteI2C_Byte(0x06,0x08); //hfp[7:0] not care
    HDMI_WriteI2C_Byte(0x07,(uint8_t)(hact >> 8)); //hactive[15:8]
    HDMI_WriteI2C_Byte(0x08,(uint8_t)(hact)); //hactive[7:0]

    HDMI_WriteI2C_Byte(0x09,(uint8_t)(vs)); //vfp[7:0]
    HDMI_WriteI2C_Byte(0x0a,0x00); //bit[3:0]:vbp[11:8]
    HDMI_WriteI2C_Byte(0x0b,(uint8_t)(vbp)); //vbp[7:0]
    HDMI_WriteI2C_Byte(0x0c,(uint8_t)(vact >> 8)); //vcat[15:8]
    HDMI_WriteI2C_Byte(0x0d,(uint8_t)(vact)); //vcat[7:0]
    HDMI_WriteI2C_Byte(0x0e,(uint8_t)(vfp >> 8)); //vfp[11:8]
    HDMI_WriteI2C_Byte(0x0f,(uint8_t)(vfp)); //vfp[7:0]
}

/******************************************************************
*Founction: ²âÊÔ Pattern	Êä³ö¡£
*¸ù¾Ý½á¹¹ÌåVideo ÖÐµÄtiming£¬ÉèÖÃVideo ºÍ pixel clk¡£
*******************************************************************/
void LT9211_Patten(struct video_timing *video_format)
{
    uint32_t pclk_khz;
    uint8_t dessc_pll_post_div = 0;
    uint32_t pcr_m, pcr_k;

    pclk_khz = video_format->pclk_khz;

    HDMI_WriteI2C_Byte(0xff,0xf9);
    HDMI_WriteI2C_Byte(0x3e,0x80);

    HDMI_WriteI2C_Byte(0xff,0x85);
    HDMI_WriteI2C_Byte(0x88,0xd0);

    HDMI_WriteI2C_Byte(0xa1,0x74);
    HDMI_WriteI2C_Byte(0xa2,0xff);

    HDMI_WriteI2C_Byte(0xa3,(uint8_t)((video_format->hs + video_format->hbp) / 256));
    HDMI_WriteI2C_Byte(0xa4,(uint8_t)((video_format->hs + video_format->hbp) % 256));//h_start

    HDMI_WriteI2C_Byte(0xa5,(uint8_t)((video_format->vs + video_format->vbp) % 256));//v_start

    HDMI_WriteI2C_Byte(0xa6,(uint8_t)(video_format->hact / 256));
    HDMI_WriteI2C_Byte(0xa7,(uint8_t)(video_format->hact % 256)); //hactive

    HDMI_WriteI2C_Byte(0xa8,(uint8_t)(video_format->vact / 256));
    HDMI_WriteI2C_Byte(0xa9,(uint8_t)(video_format->vact % 256));  //vactive

    HDMI_WriteI2C_Byte(0xaa,(uint8_t)(video_format->htotal / 256));
    HDMI_WriteI2C_Byte(0xab,(uint8_t)(video_format->htotal % 256));//htotal

    HDMI_WriteI2C_Byte(0xac,(uint8_t)(video_format->vtotal / 256));
    HDMI_WriteI2C_Byte(0xad,(uint8_t)(video_format->vtotal % 256));//vtotal

    HDMI_WriteI2C_Byte(0xae,(uint8_t)(video_format->hs / 256));
    HDMI_WriteI2C_Byte(0xaf,(uint8_t)(video_format->hs % 256));   //hsa

    HDMI_WriteI2C_Byte(0xb0,(uint8_t)(video_format->vs % 256));    //vsa

    //dessc pll to generate pixel clk
    HDMI_WriteI2C_Byte(0xff,0x82); //dessc pll
    HDMI_WriteI2C_Byte(0x2d,0x48); //pll ref select xtal

    if (pclk_khz < 44000) {
        HDMI_WriteI2C_Byte(0x35,0x83);
        dessc_pll_post_div = 16;
    } else if (pclk_khz < 88000) {
        HDMI_WriteI2C_Byte(0x35,0x82);
        dessc_pll_post_div = 8;
    } else if (pclk_khz < 176000) {
        HDMI_WriteI2C_Byte(0x35,0x81);
        dessc_pll_post_div = 4;
    } else if (pclk_khz < 352000) {
        HDMI_WriteI2C_Byte(0x35,0x80);
        dessc_pll_post_div = 0;
    }

    pcr_m = (pclk_khz * dessc_pll_post_div) / 25;
    pcr_k = pcr_m % 1000;
    pcr_m = pcr_m / 1000;

    pcr_k <<= 14;

    //pixel clk
    HDMI_WriteI2C_Byte(0xff,0xd0); //pcr
    HDMI_WriteI2C_Byte(0x2d,0x7f);
    HDMI_WriteI2C_Byte(0x31,0x00);

    HDMI_WriteI2C_Byte(0x26,0x80|((uint8_t)pcr_m));
    HDMI_WriteI2C_Byte(0x27,(uint8_t)((pcr_k >> 16) & 0xff)); //K
    HDMI_WriteI2C_Byte(0x28,(uint8_t)((pcr_k >> 8) & 0xff)); //K
    HDMI_WriteI2C_Byte(0x29,(uint8_t)(pcr_k & 0xff)); //K
}

void LT9211_SetTxTimingpattern(struct video_timing *video_format)
{
    vs = video_format->vs;
    hs = video_format->hs;
    vbp = video_format->vbp;
    vfp = video_format->vfp;
    hbp = video_format->hbp;
    hfp = video_format->hfp;
    vtotal = video_format->vtotal;
    htotal = video_format->htotal;
    vact = video_format->vact;
    hact = video_format->hact;

    HDMI_WriteI2C_Byte(0xff,0xd4);
    HDMI_WriteI2C_Byte(0x04,0x08); //hs[7:0] not care
    HDMI_WriteI2C_Byte(0x05,0x08); //hbp[7:0] not care
    HDMI_WriteI2C_Byte(0x06,0x08); //hfp[7:0] not care
    HDMI_WriteI2C_Byte(0x07,(uint8_t)(hact >> 8)); //hactive[15:8]
    HDMI_WriteI2C_Byte(0x08,(uint8_t)(hact)); //hactive[7:0]

    HDMI_WriteI2C_Byte(0x09,(uint8_t)(vs)); //vfp[7:0]
    HDMI_WriteI2C_Byte(0x0a,0x00); //bit[3:0]:vbp[11:8]
    HDMI_WriteI2C_Byte(0x0b,(uint8_t)(vbp)); //vbp[7:0]
    HDMI_WriteI2C_Byte(0x0c,(uint8_t)(vact >> 8)); //vcat[15:8]
    HDMI_WriteI2C_Byte(0x0d,(uint8_t)(vact)); //vcat[7:0]
    HDMI_WriteI2C_Byte(0x0e,(uint8_t)(vfp >> 8)); //vfp[11:8]
    HDMI_WriteI2C_Byte(0x0f,(uint8_t)(vfp)); //vfp[7:0]
}

void LT9211_Config(void)
{
    LT9211_ChipID();
    LT9211_SystemInt();
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

void LT9211_MainLoop(void)
{
    static int flag_lvds_chg = 1;
    if (lt9211_lvds_clkstb_check()) {
        if (flag_lvds_chg) {
            DISPSS_LOG_INFO("lvds clk stable\n");
            LT9211_ClockCheckDebug();
            LT9211_LvdsRxPll();
            lt9211_vid_chk_rst();              //video chk soft rst
            lt9211_lvdsrx_logic_rst();
            mdelay(100);
            LT9211_VideoCheck();

            //mipi tx set
            LT9211_MipiTxPhy();
            LT9211_MipiTxpll();
            LT9211_SetTxTiming();
            InitPanel();
            LT9211_MipiTxDigital();

            flag_lvds_chg = 0;
        }
    } else {
        if (!flag_lvds_chg) {
            DISPSS_LOG_INFO("lvds clk not stable\n");
            flag_lvds_chg = 1;
        }
    }
    //Lt9211 mipi test
    /* LT9211_MipiTxpll();
    LT9211_MipiTxPhy();
    LT9211_Patten(&video_480x1920_60Hz);
    LT9211_SetTxTimingpattern(&video_480x1920_60Hz);
    InitPanel();
    LT9211_MipiTxDigital(); */
    mdelay(3000);
    Backlight_Set();
}

void LT9211_Main(void)
{
    LT9211_Reset();
    LT9211_Config();
}
