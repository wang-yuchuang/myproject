#include <can_proxy.h>
#include <rpmsg_port.h>
#include <CLI_type.h>
#include <debug.h>

void start_can_proxy(void)
{
    if (!can_proxy_buffer_init()) {
        CPDBG("proxy buffer init fail.\n");
        return;
    }

    CPDBG("proxy buffer init done\n");

    // set can interface with no bind rpmsg_if
    for (int i = 0; i < 4; i++) {
        cp_interface_setting(cpifBusCan,
                             i,
                             can_if_init,
                             can_if_tx,
                             can_if_rx_cb,
                             NULL, 0);
    }

    CPDBG("can interface setting done\n");
    //enbale safety to get ipcc message from remote
    rpmsg_if_start_manager();
    CPDBG("set the callback to control interface done\n");
    // to init the can interface and create thread to recieve data on safety
    cp_interface_start();
}

static int can_proxy(int argc, char *argv[])
{
    ssdk_printf(SSDK_INFO, "starting can proxy service\n");
    start_can_proxy();
    return 0;
}

CLI_CMD("can_proxy", "starting can proxy service", can_proxy);