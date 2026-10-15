/**
 * @file cam_cfg.c
 * @brief external camera resource configurations.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include "cam_cfg.h"
#include "driver/kunlun/gpio/tca6408.h"
#include "driver/kunlun/gpio/tca9539.h"
#include <i2c_cfg.h>

char dev_name0[] = "serdes_dev";
char dev_name1[] = "serdes_dev";
char dev_name2[] = "serdes_dev";

char serdes_module_name0[] = "max96722_sengyun_IMX390c_5200_GMSL2_1080p";
char serdes_module_name1[] = "max96722_fengjing_MC163_MAX96705_720p";
char serdes_module_name2[] = "max96706_fengjing_MC163_MAX96705_720p";

char extgpio_tca9539[] = "tca9539";
char extgpio_tca6408[] = "tca6408";

/* camera initialization */
void board_camera_init(void)
{
    enum camera_id id;
    enum mipicsi_id mipi_id;
    bool csi1 = false;
    bool csi2 = false;
    bool csi3 = false;
    bool mipi_csi1 = false;
    bool mipi_csi2 = false;

    if (mipi_csi1) {
        mipi_id = MIPI_CSI0;
        g_mipi_cfg[mipi_id].sync = 1;
        g_mipi_cfg[mipi_id].lane_num = 4;
        g_mipi_cfg[mipi_id].lanerate = 1000 * 1000 * 1000;
        g_mipi_cfg[mipi_id].hsa = 10;
        g_mipi_cfg[mipi_id].hbp = 20;
        g_mipi_cfg[mipi_id].hsd = 0x60;
        g_mipi_cfg[mipi_id].output_type = 0x1E;
    }

    if (mipi_csi2) {
        mipi_id = MIPI_CSI1;
        g_mipi_cfg[mipi_id].sync = 0;
        g_mipi_cfg[mipi_id].lane_num = 4;
        g_mipi_cfg[mipi_id].lanerate = 1500 * 1000 * 1000;
        g_mipi_cfg[mipi_id].hsa = 10;
        g_mipi_cfg[mipi_id].hbp = 20;
        g_mipi_cfg[mipi_id].hsd = 0x60;
        g_mipi_cfg[mipi_id].output_type = 0x1E;
    }

    if (csi1) {
        id = CAMERA_0;
        g_cam_cfg[id].power_gpio = TCA6408_P02;
        g_cam_cfg[id].poc_gpio = TCA6408_P07;
        g_cam_cfg[id].i2c_handle =  &g_i2c9_adap_dev;
        g_cam_cfg[id].i2c_addr = 0x6b;
        g_cam_cfg[id].poc_i2c_addr = 0x28;
        g_cam_cfg[id].gpioext_addr = 0x20;
        g_cam_cfg[id].ser_i2c_addr = 0x40;
        g_cam_cfg[id].width = 1280;
        g_cam_cfg[id].height = 720;
        g_cam_cfg[id].name = dev_name0;
        g_cam_cfg[id].module_name = serdes_module_name1;
        g_cam_cfg[id].mipi_cfg = &g_mipi_cfg[MIPI_CSI0];
        g_cam_cfg[id].gpioext_name = extgpio_tca6408;
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
        cam_confs[id].enabled = true;
#endif

        if (g_mipi_cfg[MIPI_CSI0].sync == 1)
            goto cam4;

        id = CAMERA_1;
        g_cam_cfg[id].power_gpio = TCA6408_P02;
        g_cam_cfg[id].poc_gpio = TCA6408_P07;
        g_cam_cfg[id].i2c_handle =  &g_i2c9_adap_dev;
        g_cam_cfg[id].i2c_addr = 0x6b;
        g_cam_cfg[id].poc_i2c_addr = 0x28;
        g_cam_cfg[id].gpioext_addr = 0x20;
        g_cam_cfg[id].ser_i2c_addr = 0x40;
        g_cam_cfg[id].width = 1280;
        g_cam_cfg[id].height = 720;
        g_cam_cfg[id].name = dev_name0;
        g_cam_cfg[id].module_name = serdes_module_name1;
        g_cam_cfg[id].mipi_cfg = &g_mipi_cfg[MIPI_CSI0];
        g_cam_cfg[id].gpioext_name = extgpio_tca6408;
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
        cam_confs[id].enabled = true;
#endif

        id = CAMERA_2;
        g_cam_cfg[id].power_gpio = TCA6408_P02;
        g_cam_cfg[id].poc_gpio = TCA6408_P07;
        g_cam_cfg[id].i2c_handle =  &g_i2c9_adap_dev;
        g_cam_cfg[id].i2c_addr = 0x6b;
        g_cam_cfg[id].poc_i2c_addr = 0x28;
        g_cam_cfg[id].gpioext_addr = 0x20;
        g_cam_cfg[id].ser_i2c_addr = 0x40;
        g_cam_cfg[id].width = 1280;
        g_cam_cfg[id].height = 720;
        g_cam_cfg[id].name = dev_name0;
        g_cam_cfg[id].module_name = serdes_module_name1;
        g_cam_cfg[id].mipi_cfg = &g_mipi_cfg[MIPI_CSI0];
        g_cam_cfg[id].gpioext_name = extgpio_tca6408;
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
        cam_confs[id].enabled = true;
#endif

        id = CAMERA_3;
        g_cam_cfg[id].power_gpio = TCA6408_P02;
        g_cam_cfg[id].poc_gpio = TCA6408_P07;
        g_cam_cfg[id].i2c_handle =  &g_i2c9_adap_dev;
        g_cam_cfg[id].i2c_addr = 0x6b;
        g_cam_cfg[id].poc_i2c_addr = 0x28;
        g_cam_cfg[id].gpioext_addr = 0x20;
        g_cam_cfg[id].ser_i2c_addr = 0x40;
        g_cam_cfg[id].width = 1280;
        g_cam_cfg[id].height = 720;
        g_cam_cfg[id].name = dev_name0;
        g_cam_cfg[id].module_name = serdes_module_name1;
        g_cam_cfg[id].mipi_cfg = &g_mipi_cfg[MIPI_CSI0];
        g_cam_cfg[id].gpioext_name = extgpio_tca6408;
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
        cam_confs[id].enabled = true;
#endif
    }
cam4:
    if (csi2) {
        id = CAMERA_4;
        g_cam_cfg[id].power_gpio = TCA9539_P02;
        g_cam_cfg[id].poc_gpio = TCA9539_P14;
        g_cam_cfg[id].i2c_handle = &g_i2c10_adap_dev; //TODO: &g_i2c10_adap_dev;
        g_cam_cfg[id].i2c_addr = 0x6b;
        g_cam_cfg[id].poc_i2c_addr = 0x29;
        g_cam_cfg[id].gpioext_addr = 0x74;
        g_cam_cfg[id].ser_i2c_addr = 0x40;
        g_cam_cfg[id].width = 1920;
        g_cam_cfg[id].height = 1080;
        g_cam_cfg[id].name = dev_name1;
        g_cam_cfg[id].module_name = serdes_module_name0;
        g_cam_cfg[id].mipi_cfg = &g_mipi_cfg[MIPI_CSI1];
        g_cam_cfg[id].gpioext_name = extgpio_tca9539;
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
        cam_confs[id].enabled = true;
#endif

        if (g_mipi_cfg[MIPI_CSI1].sync == 1)
            goto cam8;

        id = CAMERA_5;
        g_cam_cfg[id].power_gpio = TCA9539_P02;
        g_cam_cfg[id].poc_gpio = TCA9539_P14;
        g_cam_cfg[id].i2c_handle = &g_i2c10_adap_dev; //TODO: &g_i2c10_adap_dev;
        g_cam_cfg[id].i2c_addr = 0x6b;
        g_cam_cfg[id].poc_i2c_addr = 0x29;
        g_cam_cfg[id].gpioext_addr = 0x74;
        g_cam_cfg[id].ser_i2c_addr = 0x40;
        g_cam_cfg[id].width = 1920;
        g_cam_cfg[id].height = 1080;
        g_cam_cfg[id].name = dev_name1;
        g_cam_cfg[id].module_name = serdes_module_name0;
        g_cam_cfg[id].mipi_cfg = &g_mipi_cfg[MIPI_CSI1];
        g_cam_cfg[id].gpioext_name = extgpio_tca9539;
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
        cam_confs[id].enabled = true;
#endif

        id = CAMERA_6;
        g_cam_cfg[id].power_gpio = TCA9539_P02;
        g_cam_cfg[id].poc_gpio = TCA9539_P14;
        g_cam_cfg[id].i2c_handle = &g_i2c10_adap_dev; //TODO: &g_i2c10_adap_dev;
        g_cam_cfg[id].i2c_addr = 0x6b;
        g_cam_cfg[id].poc_i2c_addr = 0x29;
        g_cam_cfg[id].gpioext_addr = 0x74;
        g_cam_cfg[id].ser_i2c_addr = 0x40;
        g_cam_cfg[id].width = 1920;
        g_cam_cfg[id].height = 1080;
        g_cam_cfg[id].name = dev_name1;
        g_cam_cfg[id].module_name = serdes_module_name0;
        g_cam_cfg[id].mipi_cfg = &g_mipi_cfg[MIPI_CSI1];
        g_cam_cfg[id].gpioext_name = extgpio_tca9539;
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
        cam_confs[id].enabled = true;
#endif

        id = CAMERA_7;
        g_cam_cfg[id].power_gpio = TCA9539_P02;
        g_cam_cfg[id].poc_gpio = TCA9539_P14;
        g_cam_cfg[id].i2c_handle = &g_i2c10_adap_dev; //TODO: &g_i2c10_adap_dev;
        g_cam_cfg[id].i2c_addr = 0x6b;
        g_cam_cfg[id].poc_i2c_addr = 0x29;
        g_cam_cfg[id].gpioext_addr = 0x74;
        g_cam_cfg[id].ser_i2c_addr = 0x40;
        g_cam_cfg[id].width = 1920;
        g_cam_cfg[id].height = 1080;
        g_cam_cfg[id].name = dev_name1;
        g_cam_cfg[id].module_name = serdes_module_name0;
        g_cam_cfg[id].mipi_cfg = &g_mipi_cfg[MIPI_CSI1];
        g_cam_cfg[id].gpioext_name = extgpio_tca9539;
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
        cam_confs[id].enabled = true;
#endif
    }
cam8:
    if (csi3) {
        id = CAMERA_8;
        g_cam_cfg[id].power_gpio = 172;//TODO: PortConf_PIN_OSPI2_DATA1;
        g_cam_cfg[id].poc_gpio = TCA9539_P06;
        g_cam_cfg[id].gpi_gpio = TCA9539_P11;
        g_cam_cfg[id].i2c_handle = &g_i2c10_adap_dev; //TODO: &g_i2c10_adap_dev;
        g_cam_cfg[id].i2c_addr = 0x78;
        g_cam_cfg[id].poc_i2c_addr = 0x29;
        g_cam_cfg[id].gpioext_addr = 0x74;
        g_cam_cfg[id].ser_i2c_addr = 0x40;
        g_cam_cfg[id].width = 1280;
        g_cam_cfg[id].height = 720;
        g_cam_cfg[id].name = dev_name2;
        g_cam_cfg[id].module_name = serdes_module_name2;
        g_cam_cfg[id].gpioext_name = extgpio_tca9539;
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
        cam_confs[id].enabled = true;
#endif
    }
}
