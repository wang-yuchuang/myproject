/**
 * @file watchdog
 * @brief watchdog common driver.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <debug.h>
#include <stdlib.h>

#include "driver/watchdog.h"

/* Returns the operations for a watchdog device */
#define device_get_ops(wdd) (wdd->ops)

int32_t watchdog_start(struct watchdog_device *wdd)
{
    struct watchdog_ops *ops = device_get_ops(wdd);

    if (!ops->start)
        return -1;

    return ops->start(wdd);
}

int32_t watchdog_stop(struct watchdog_device *wdd)
{
    struct watchdog_ops *ops = device_get_ops(wdd);

    if (!ops->stop)
        return -1;

    return ops->stop(wdd);
}

int32_t watchdog_ping(struct watchdog_device *wdd)
{
    struct watchdog_ops *ops = device_get_ops(wdd);

    if (!ops->ping)
        return -1;

    return ops->ping(wdd);
}

int32_t watchdog_set_timeout(struct watchdog_device *wdd, uint32_t timeout)
{
    struct watchdog_ops *ops = device_get_ops(wdd);

    if (!ops->set_timeout)
        return -1;

    return ops->set_timeout(wdd, timeout);
}

int32_t watchdog_get_timeleft(struct watchdog_device *wdd, uint32_t *timeleft)
{
    struct watchdog_ops *ops = device_get_ops(wdd);

    *timeleft = 0;

    if (!ops->get_timeleft)
        return -1;

    *timeleft = ops->get_timeleft(wdd);

    return 0;
}