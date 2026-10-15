/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <sys/types.h>


#include "ulink.h"


#if 0
static void ulink_init(const struct app_descriptor *app)
{
    ulink_irq_init();
    ulink_channel_init();
}

#endif
void ulink_service_init()
{
    ulink_irq_init();

   ulink_channel_init();

   ulink_net_init();


}


//APP_START(ulink)
//.init = (app_init)ulink_init,
//APP_END

