/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <debug.h>
#include <string.h>
#include <CLI.h>
#include <debug.h>
#include <stdlib.h>

#include <driver/kunlun/pvt/pvt.h>
#include <pvt_cfg.h>
#include "pvt_app.h"
#define LOCAL_TRACE 0 //close local trace 1->0

#define PVT_PRINT_WAIT_TIME CONFIG_PVT_PRINT_WAIT_TIME // seconds


#define PVT_PRINT_WAIT_TIME_VALUE (PVT_PRINT_WAIT_TIME*1000)

#define PVT_MODE_TYPE_TEMP 0
#define PVT_MODE_TYPE_VOL 1
#define PVT_MODE_TYPE_P 2

#define PVT_DEVICE_TYPE_LVT 1
#define PVT_DEVICE_TYPE_ULVT 2
#define PVT_DEVICE_TYPE_SVT 3

#define PVT_OUT_TYPE_OFF 0
#define PVT_OUT_TYPE_ON_ALL 1
#define PVT_OUT_TYPE_ON_ONE_TYPE 2


#define PVT_RES_ID_ALL PVT_DEV_INS_NUM

//@NOTE config option
#ifdef CONFIG_SAFETY
#define PVT_RES_ID_ PVT_DEV_SAFETY_INS
#endif /* end CONFIG_SAFETY */

#ifdef CONFIG_MP
#define PVT_RES_ID_ PVT_DEV_AP_INS
#endif /* end CONFIG_MP */

#ifndef PVT_RES_ID_
#define PVT_RES_ID_ PVT_DEV_SAFETY_INS
#endif /* PVT_RES_ID_ */

typedef struct pvt_ctrl_data
{
    int pvt_out_print;
    int pvt_mode_type;
    int pvt_device_type;
    int domain_type;
}pvt_ctrl_data_t;

pvt_ctrl_data_t g_out_data = {0};
static char domain_name[PVT_RES_ID_ALL][8]={{"safety"},{"secure"}};


static pvt_dev_t* get_pvt_dev(int32_t devId)
{
    if (devId >= PVT_DEV_INS_NUM) {
        ssdk_printf(SSDK_EMERG, "parameter error for %s.\n", __func__);
        return NULL;
    }

    return g_pvt_dev_ptr[devId];
}

static char* get_pvt_mode(int32_t mode)
{
    if (mode == PVT_MODE_TYPE_TEMP) {
        return "temp";
    }
    else if (mode == PVT_MODE_TYPE_VOL) {
        return "voltage";
    }
    else if (mode == PVT_MODE_TYPE_P) {
        return "process";
    }
    else {
        return "invalid";
    }
}

static char* get_pvt_type(int32_t type)
{
    if (type == PVT_DEVICE_TYPE_LVT) {
        return "lvt";
    }
    else if (type == PVT_DEVICE_TYPE_ULVT) {
        return "ulvt";
    }
    else if (type == PVT_DEVICE_TYPE_SVT) {
        return "svt";
    }
    else {
        return "invalid";
    }
}

int pvt_ctrl(void* args)
{

    int ret = 0;
    int i = 0;
    pvt_data_t out_data;
    int32_t out_p_data = 0;
    float out_v_data;
    float out_t_data;
#ifdef CONFIG_PVT_AUTO_OUT_PRT
    g_out_data.pvt_out_print = PVT_OUT_TYPE_ON_ALL;
    g_out_data.pvt_device_type = PVT_DEVICE_TYPE_ULVT;
    g_out_data.domain_type = PVT_RES_ID_;
#endif /* end CONFIG_PVT_AUTO_OUT_PRT */
    ssdk_printf(SSDK_EMERG, "%s entry\n", __func__);
    //read pvt value
    while (1) {

        if (g_out_data.pvt_out_print == PVT_OUT_TYPE_ON_ALL) {

            if (g_out_data.domain_type == PVT_RES_ID_ALL) {
                for (i = PVT_DEV_SAFETY_INS; i < PVT_RES_ID_ALL; i++) {

                    if (pvt_get_pvt(get_pvt_dev(i), &out_data) == 0) {
                        ssdk_printf(SSDK_EMERG, "%s temperature value = %5.2f, voltage value = %3.2f, %s process value = %d\n"
                                            , domain_name[i]
                                            , out_data.temp
                                            , out_data.voltage
                                            , get_pvt_type(g_out_data.pvt_device_type)
                                            , out_data.process);
                    }
                    else {
                        ssdk_printf(SSDK_EMERG, "%s pvt device is invalid for get pvt val\n", domain_name[i]);
                    }
                }
            }else{

                if (pvt_get_pvt(get_pvt_dev(g_out_data.domain_type), &out_data) == 0) {
                    ssdk_printf(SSDK_EMERG, "%s temperature value = %5.2f, voltage value = %3.2f, %s process value = %d\n"
                                            , domain_name[g_out_data.domain_type]
                                            , out_data.temp
                                            , out_data.voltage
                                            , get_pvt_type(g_out_data.pvt_device_type)
                                            , out_data.process);
                }
                else {
                    ssdk_printf(SSDK_EMERG, "%s pvt device is invalid for get pvt val\n", domain_name[g_out_data.domain_type]);
                }
            }
            osDelay(PVT_PRINT_WAIT_TIME_VALUE);
        }
        else if (g_out_data.pvt_out_print == PVT_OUT_TYPE_ON_ONE_TYPE) {
            if (g_out_data.domain_type == PVT_RES_ID_ALL) {
                for (i = PVT_DEV_SAFETY_INS; i < PVT_RES_ID_ALL; i++) {
                    if (g_out_data.pvt_mode_type == PVT_MODE_TYPE_TEMP) {

                        if (pvt_float_data_get(get_pvt_dev(i), PVT_GET_FLOAT_CUR_TEMP, -1, &out_t_data) == 0) {
                            ssdk_printf(SSDK_EMERG, "%s temperature value = %5.2f\n", domain_name[i], out_t_data);
                        }
                        else {
                            ssdk_printf(SSDK_EMERG, "%s pvt device is invalid for get temp\n", domain_name[i]);
                        }

                    }
                    else if (g_out_data.pvt_mode_type == PVT_MODE_TYPE_VOL) {
                        if (pvt_float_data_get(get_pvt_dev(i), PVT_GET_FLOAT_CUR_VOLTAGE, -1, &out_v_data) == 0) {
                            ssdk_printf(SSDK_EMERG, "%s voltage value = %3.2f\n", domain_name[i], out_v_data);
                        }
                        else {
                            ssdk_printf(SSDK_EMERG, "%s pvt device is invalid for get voltage\n", domain_name[i]);
                        }

                    }
                    else if (g_out_data.pvt_mode_type == PVT_MODE_TYPE_P) {
                        if (pvt_integer_data_get(get_pvt_dev(i), PVT_GET_INT_PROCESS, -1, &out_p_data) == 0) {
                            ssdk_printf(SSDK_EMERG, "%s %s process value = %d\n"
                                                , domain_name[i]
                                                , get_pvt_type(g_out_data.pvt_device_type)
                                                , out_p_data);
                        }
                        else {
                            ssdk_printf(SSDK_EMERG, "%s pvt device is invalid for get process\n", domain_name[i]);
                        }
                    }
                    else {
                        // do nothing
                    }
                }
            }
            else {
                if (g_out_data.pvt_mode_type == PVT_MODE_TYPE_TEMP) {
                    if (pvt_float_data_get(get_pvt_dev(g_out_data.domain_type), PVT_GET_FLOAT_CUR_TEMP, -1, &out_t_data) == 0) {
                        ssdk_printf(SSDK_EMERG, "%s temperature value = %5.2f\n", domain_name[g_out_data.domain_type], out_t_data);
                    }
                    else {
                        ssdk_printf(SSDK_EMERG, "%s pvt device is invalid for get temp\n", domain_name[g_out_data.domain_type]);
                    }
                }
                else if (g_out_data.pvt_mode_type == PVT_MODE_TYPE_VOL) {
                    if (pvt_float_data_get(get_pvt_dev(g_out_data.domain_type), PVT_GET_FLOAT_CUR_VOLTAGE, -1, &out_v_data) == 0) {
                        ssdk_printf(SSDK_EMERG, "%s voltage value = %3.2f\n", domain_name[g_out_data.domain_type], out_v_data);
                    }
                    else {
                        ssdk_printf(SSDK_EMERG, "%s pvt device is invalid for get voltage\n", domain_name[g_out_data.domain_type]);
                    }
                }
                else if (g_out_data.pvt_mode_type == PVT_MODE_TYPE_P) {
                    if (pvt_integer_data_get(get_pvt_dev(g_out_data.domain_type), PVT_GET_INT_PROCESS, -1, &out_p_data) == 0) {
                        ssdk_printf(SSDK_EMERG, "%s  %s process value = %d\n"
                                            , domain_name[g_out_data.domain_type]
                                            , get_pvt_type(g_out_data.pvt_device_type)
                                            , out_p_data);
                    }
                    else {
                        ssdk_printf(SSDK_EMERG, "%s pvt device is invalid for get process\n", domain_name[g_out_data.domain_type]);
                    }
                }
                else {
                    // do nothing
                }
            }

            osDelay(PVT_PRINT_WAIT_TIME_VALUE);
        }else{
            osDelay(PVT_PRINT_WAIT_TIME_VALUE);
        }
    }

    return ret;
}

int pvt_set_print(int argc, char* argv[])
{
    int32_t is_out = 0;
/**
 * @param argv[0] on/off
 */
    if (argc == 1) {
        is_out = atoi(argv[0]);

        if (is_out == 0) {
            g_out_data.pvt_out_print = PVT_OUT_TYPE_OFF;
        }
        else if (is_out == 1) {

            g_out_data.pvt_out_print = PVT_OUT_TYPE_ON_ALL;
            g_out_data.pvt_device_type = PVT_DEVICE_TYPE_ULVT;
            g_out_data.domain_type = PVT_RES_ID_;
        }
        else {
            ssdk_printf(SSDK_EMERG, "input argv error\n");
            return CLI_FALSE;
        }
    }
/**
 * @param argv[0] off/ulvt/lvt/svt
 * @param argv[1] safety/ap/all
 */
    else if (argc == 2) {
        is_out = atoi(argv[0]);
        g_out_data.domain_type = atoi(argv[1]);

        if ((is_out == PVT_DEVICE_TYPE_ULVT) ||
            (is_out == PVT_DEVICE_TYPE_LVT) ||
            (is_out == PVT_DEVICE_TYPE_SVT)) {
            if((g_out_data.domain_type < PVT_DEV_SAFETY_INS) ||
               (g_out_data.domain_type > PVT_RES_ID_ALL)) {
                ssdk_printf(SSDK_EMERG, "input argv error\n");
                return CLI_FALSE;
            }

            g_out_data.pvt_out_print = PVT_OUT_TYPE_ON_ALL;
            g_out_data.pvt_device_type = is_out;
        }
        else if (is_out == 0) {
            g_out_data.pvt_out_print = PVT_OUT_TYPE_OFF;
        }
        else {
            ssdk_printf(SSDK_EMERG, "input argv error\n");
            return CLI_FALSE;
        }

    }
/**
 * @param argv[0] P/V/T
 * @param argv[1] ulvt/lvt/svt
 * @param argv[2] safety/ap/all
 */
    else if (argc == 3) {
        g_out_data.pvt_mode_type = atoi(argv[0]);
        g_out_data.pvt_device_type = atoi(argv[1]);
        g_out_data.domain_type = atoi(argv[2]);

        if (((g_out_data.domain_type < PVT_DEV_SAFETY_INS) || (g_out_data.domain_type > PVT_RES_ID_ALL)) ||
            ((g_out_data.pvt_mode_type < PVT_MODE_TYPE_TEMP) || (g_out_data.pvt_mode_type > PVT_MODE_TYPE_P)) ||
            ((g_out_data.pvt_mode_type == PVT_MODE_TYPE_P) && ((g_out_data.pvt_device_type < PVT_DEVICE_TYPE_LVT) ||
             (g_out_data.pvt_device_type > PVT_DEVICE_TYPE_SVT)))) {
            ssdk_printf(SSDK_EMERG, "input argv error\n");
            return CLI_FALSE;
        }

        g_out_data.pvt_out_print = PVT_OUT_TYPE_ON_ONE_TYPE;
    }
    else {
        ssdk_printf(SSDK_EMERG, "input argv error\n");
        return CLI_FALSE;
    }

    if (g_out_data.domain_type == PVT_RES_ID_ALL) {
        for (int32_t i = 0; i < PVT_RES_ID_ALL; i++) {
            // set device type and sensor mode
            pvt_integer_data_set(get_pvt_dev(i), PVT_SET_INT_SAMPLE_MODE, g_out_data.pvt_mode_type);
            //@NOTE need convert pvt_device_type value to enum
            pvt_integer_data_set(get_pvt_dev(i), PVT_SET_INT_SENSOR_PNTYPE, g_out_data.pvt_device_type - 1);
            ssdk_printf(SSDK_EMERG, "##%s print config: \n -print status: %s\n -sensor mode: %s\n -sensor type: %s\n"
                                  , domain_name[i]
                                  , g_out_data.pvt_out_print == 0 ? "off" : "on"
                                  , get_pvt_mode(g_out_data.pvt_mode_type)
                                  , get_pvt_type(g_out_data.pvt_device_type));
        }
    }
    else {
        // set device type and sensor mode
        pvt_integer_data_set(get_pvt_dev(g_out_data.domain_type), PVT_SET_INT_SAMPLE_MODE, g_out_data.pvt_mode_type);
        //@NOTE need convert pvt_device_type value to enum
        pvt_integer_data_set(get_pvt_dev(g_out_data.domain_type), PVT_SET_INT_SENSOR_PNTYPE, g_out_data.pvt_device_type - 1);
        ssdk_printf(SSDK_EMERG, "##%s print config: \n -print status: %s\n -sensor mode: %s\n -sensor type: %s\n"
                                , domain_name[g_out_data.domain_type]
                                , g_out_data.pvt_out_print == 0 ? "off" : "on"
                                , get_pvt_mode(g_out_data.pvt_mode_type)
                                , get_pvt_type(g_out_data.pvt_device_type));
    }
    return 0;
}

int32_t pvt_app_print_init()
{
    //pvt setting
    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "pvt_print";
    attr.priority = osPriorityNormal;
    attr.stack_size = 1024;

    osThreadNew((osThreadFunc_t)pvt_ctrl, NULL, &attr);

    return 0;
}


#ifdef CONFIG_CLI
CLI_CMD("pvt_on", "print pvt info\r\n\t\
pvt_on (0/1/2/3) (0/1/2): print p v t, (off/ulvt/lvt/svt) (saf sec all)\r\n\t\
pvt_on (0/1/2) (1/2/3) (0/1/2): print p/v/t, (t/v/p) (ulvt/lvt/svt) (saf sec all)", pvt_set_print);
#endif /* end  CONFIG_CLI*/

