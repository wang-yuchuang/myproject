/**
 * @file disp_interface.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <disp_interface.h>
#include <disp_drv_log.h>

extern DISP_MODULE_DRIVER disp_driver_lvds;
extern DISP_MODULE_DRIVER disp_driver_dsi;
extern DISP_MODULE_DRIVER disp_driver_parallel;

static DISP_MODULE_DRIVER *if_module_driver[DISP_IF_MODULE_NUM] = {
    &disp_driver_lvds,
    &disp_driver_dsi,
    &disp_driver_parallel,
};

static const char* disp_get_module_name(DISP_IF_MODULE_ENUM module)
{
    switch (module) {
    case DISP_IF_MODULE_LVDS: return "lvds";
    case DISP_IF_MODULE_DSI:  return "dsi";
    case DISP_IF_MODULE_PARALLEL: return "parallel";
    default:
        DISPERR("invalid module id = %d\n", module);
        return "unknown";
    }
}

int disp_interface_init(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev)
{
    if (module == DISP_IF_MODULE_UNKNOWN) {
        DISPERR("init module is unknown\n");
        return -1;
    }

    if (if_module_driver[module]->init != 0) {
        DISPDBG("init module %s\n", disp_get_module_name(module));
        if_module_driver[module]->init(dev);
    }

    return 0;
}

int disp_interface_deinit(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev)
{
    if (module == DISP_IF_MODULE_UNKNOWN) {
        DISPERR("deinit module is unknown\n");
        return -1;
    }

    if (if_module_driver[module]->deinit != 0) {
        DISPDBG("deinit module %s\n", disp_get_module_name(module));
        if_module_driver[module]->deinit(dev);
    }

    return 0;
}

int disp_interface_reset(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev)
{
    if (module == DISP_IF_MODULE_UNKNOWN) {
        DISPERR("reset module is unknown\n");
        return -1;
    }

    if (if_module_driver[module]->reset != 0) {
        DISPDBG("reset module %s\n", disp_get_module_name(module));
        if_module_driver[module]->reset(dev);
    }
    return 0;
}

int disp_interface_config(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev, struct sdm_panel *panel)
{
    if (module == DISP_IF_MODULE_UNKNOWN) {
        DISPERR("config module is unknown\n");
        return -1;
    }

    if (if_module_driver[module]->config != 0) {
        DISPDBG("config module %s\n", disp_get_module_name(module));
        if_module_driver[module]->config(dev, panel);
    }
    return 0;
}

int disp_interface_power_on(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev)
{
    if (module == DISP_IF_MODULE_UNKNOWN) {
        DISPERR("power on module is unknown\n");
        return 0;
    }

    if (if_module_driver[module]->power_on != 0) {
        DISPDBG("power on module %s\n", disp_get_module_name(module));
        if_module_driver[module]->power_on(dev);
    }
    return 0;
}

int disp_interface_power_off(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev)
{
    if (module == DISP_IF_MODULE_UNKNOWN) {
        DISPERR("power off module is unknown\n");
        return 0;
    }

    if (if_module_driver[module]->power_off != 0) {
        DISPDBG("power off module %s\n", disp_get_module_name(module));
        if_module_driver[module]->power_off(dev);
    }
    return 0;
}