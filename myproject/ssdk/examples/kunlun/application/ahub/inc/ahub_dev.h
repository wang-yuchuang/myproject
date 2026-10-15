/** Audio Hub device interface*/
#ifndef __AHUB_DEV_H__
#define __AHUB_DEV_H__
#include "driver/snd_driver.h"
typedef enum {
    SAF_SND_DEV_TX_PARAM = 0,
    SAF_SND_DEV_RX_PARAM,
    AP_MAIN_DEV_TX_PARAM,
    AP_MAIN_DEV_RX_PARAM,
    AHUB_SND_DEV_PARAM_NUMB,
}AHUB_SND_DEV_PARAM;

int ahub_safety_dev_init(snd_dev_t * dev, snd_params_t * tx_param, snd_params_t * rx_param);
int ahub_ap_main_dev_init(snd_dev_t * dev, snd_params_t * tx_param, snd_params_t * rx_param);
extern snd_params_t * p_ahub_snd_params[AHUB_SND_DEV_PARAM_NUMB];

#endif