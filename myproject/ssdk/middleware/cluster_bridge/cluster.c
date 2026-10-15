/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <rpmsg.h>
#include <rpmsg_channel.h>
#include <rpmsg_echo.h>
#include <rpmsg_service.h>
#include <cluster_config.h>
#include "CLI_type.h"

/* Check DCF endpoint management document for allocation */
#define CLUSTER_APP_EPT_NAME           "rpmsg-cluster"
#ifndef CLUSTER_APP_EPT
#define CLUSTER_APP_EPT                (70)
#endif

#define IVI_APP_EPT_NAME               "rpmsg-ivi"
#ifndef IVI_APP_EPT
#define IVI_APP_EPT                    (71)
#endif

#define WAIT_RPMSG_DEV_TIMEOUT         (10000)
struct clusterapp_server_info {
    bool binitialized;

    /* communication level stuff */
    struct ipcc_channel *rpchan_ivi;
    struct ipcc_channel *rpchan_cluster;

    /* TODO: operation stuff */
};

struct clusterapp_cmd_msg {
    struct dcf_message head;

    /* to extend cmd body */
    uint32_t subcmd;
    uint32_t arg1;
    uint32_t arg2;
} __attribute__((__packed__));

static struct clusterapp_server_info clusterapp_server;

static int process_ivi_message(void *data, int len, unsigned long src, void *arg)
{
    struct clusterapp_server_info *s = &clusterapp_server;
    int ret = 0;

    ssdk_printf(SSDK_INFO, "clusterapp forward %d bytes to cluster\n", len);
    /* send msg */
    ret = rpmsg_channel_send(s->rpchan_cluster, IVI_APP_EPT, (char *)data, len, 1000);
    if (ret) {
        ssdk_printf(SSDK_CRIT, "process_ivi_message: send data error, ret=%d\n",
                    ret);
    }
    return ret;
}

static int process_cluster_message(void *data, int len, unsigned long src, void *arg)
{
    struct clusterapp_server_info *s = &clusterapp_server;
    int ret = 0;

    ssdk_printf(SSDK_INFO, "clusterapp forward %d bytes to ivi\n", len);
    ret = rpmsg_channel_send(s->rpchan_ivi, CLUSTER_APP_EPT, (char *)data, len, 1000);
    if (ret) {
        ssdk_printf(SSDK_CRIT, "process_cluster_message: send data error, ret=%d\n",
                    ret);
    }
    return ret;
}

void clusterapp_bridge_init(void)
{
    struct clusterapp_server_info *s = &clusterapp_server;
    struct rpmsg_instance *dev;
    int ret = 0;

    if (s->binitialized)
        return;

    /* get a rpmsg instance */
    dev = rpmsg_get_instance(RPMSG_TYPE_IPCC, DP_CA_AP1);
    if (!dev) {
        ssdk_printf(SSDK_CRIT,
                    "cluster bridge init: failed to create an rpmsg device\n");
        return;
    }

    /* create a rpmsg channel */
    s->rpchan_ivi = rpmsg_channel_create(dev, CLUSTER_APP_EPT_NAME,
                          CLUSTER_APP_EPT);
    if (s->rpchan_ivi == NULL) {
        ssdk_printf(SSDK_CRIT, "cluster bridge init: failed to create rpmsg channel\n");
        return;
    }

    /* start the rpmsg channel */
    ret = rpmsg_channel_start(s->rpchan_ivi, process_ivi_message,
                              NULL);
    if (ret != 0) {
        ssdk_printf(SSDK_CRIT, "cluster bridge init: start rpmsg channel failed.\n");
        rpmsg_channel_destroy(s->rpchan_ivi);
        return;
    }

    /* get a rpmsg instance */
    dev = rpmsg_get_instance(RPMSG_TYPE_IPCC, DP_CA_AP2);
    if (!dev) {
        ssdk_printf(SSDK_CRIT,
                    "cluster bridge init: failed to create an rpmsg device\n");
        return;
    }

    /* create a rpmsg channel */
    s->rpchan_cluster = rpmsg_channel_create(dev, IVI_APP_EPT_NAME,
                          IVI_APP_EPT);
    if (s->rpchan_cluster == NULL) {
        ssdk_printf(SSDK_CRIT, "cluster bridge init: failed to create rpmsg channel\n");
        return;
    }

    /* start the rpmsg channel */
    ret = rpmsg_channel_start(s->rpchan_cluster, process_cluster_message,
                              NULL);
    if (ret != 0) {
        ssdk_printf(SSDK_CRIT, "cluster bridge init: start rpmsg channel failed.\n");
        rpmsg_channel_destroy(s->rpchan_cluster);
        return;
    }

    ssdk_printf(SSDK_INFO, "cluster bridge init: successed.\n");
    s->binitialized = true;
}

static int cluster_bridge(int argc, char *argv[])
{
    clusterapp_bridge_init();
    return 0;
}

CLI_CMD("cluster_bridge", "cluster bridge init", cluster_bridge);