/**
 * @file cam_cfg.c
 * @brief external camera resource configurations.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifdef CONFIG_CSI

#include "cam_cfg.h"

#include "driver/taishan/pinctrl/taishan_pinctrl.h"
#include "pinmux_cfg.h"
#include "i2c_cfg.h"

char deser_max96706[] = "max96706";
char deser_ti926[] = "ti926";
char vcam_in[] = "vcamera";

/* camera initialization */
void board_camera_init(void)
{
    enum camera_id id;

    id = CAMERA_0;
    g_cam_cfg[id].gpio[0] = 0;
    g_cam_cfg[id].gpio[1] = 0;
    g_cam_cfg[id].gpio[2] = GPIO_D36; /* for camera poc */
    g_cam_cfg[id].gpio[3] = 0;
    g_cam_cfg[id].i2c_res = 0;
    g_cam_cfg[id].i2c_addr = 0x78;
    g_cam_cfg[id].name = &deser_max96706[0];
#ifdef CONFIG_I2C
    g_cam_cfg[id].i2c_handle = (void *)i2c3_adap_dev;
#endif

    id = CAMERA_1;
    g_cam_cfg[id].gpio[0] = GPIO_C14; /* gpio_pwn */
    g_cam_cfg[id].gpio[1] = GPIO_C13; /* gpio_intn */
    g_cam_cfg[id].gpio[2] = 0;
    g_cam_cfg[id].gpio[3] = 0;
    g_cam_cfg[id].i2c_res = 0;
    g_cam_cfg[id].i2c_addr = 0x2c;
    g_cam_cfg[id].name = &deser_ti926[0];
#ifdef CONFIG_I2C
    g_cam_cfg[id].i2c_handle = (void *)i2c6_adap_dev;
#endif

    id = CAMERA_2;
    g_cam_cfg[id].gpio[0] = 0;
    g_cam_cfg[id].gpio[1] = 0;
    g_cam_cfg[id].gpio[2] = 0;
    g_cam_cfg[id].gpio[3] = 0;
    g_cam_cfg[id].i2c_res = 0;
    g_cam_cfg[id].i2c_addr = 0;
    g_cam_cfg[id].name = &vcam_in[0];
    g_cam_cfg[id].i2c_handle = NULL;
}

#ifdef CONFIG_CSI_IOMUX
extern pinctrl_t g_pinctrl_safety;
void csi_iomux_cfg(int sel)
{
    unsigned int pin;
    pinctrl_t *pinctrl;

    if (sel == 1)
        goto cfg_b;

    pinctrl = &g_pinctrl_safety;
	/* for camera deser in */
    for (pin = GPIO_H0; pin <= GPIO_H10; pin++) {
        pinctrl_config(pinctrl, pin, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT3);
    }

    return;

cfg_b:

    pinctrl = &g_pinctrl_safety;
	/* for video in */
    for (pin = GPIO_B1; pin <= GPIO_C12; pin++) {
        pinctrl_config(pinctrl, pin, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT7);
    }

    return;
}
#endif
#endif
