/**
 * @file watchdog.h
 * @brief watchdog driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <debug.h>
#include <types.h>

struct watchdog_device;

/** struct watchdog_ops - The watchdog-devices operations
 *
 * @start:	The routine for starting the watchdog device.
 * @stop:	The routine for stopping the watchdog device.
 * @ping:	The routine that sends a keepalive ping to the watchdog device.
 * @set_timeout:The routine for setting the watchdog devices timeout value (in seconds).
 * @get_timeleft:The routine that gets the time left before a reset (in seconds).
 *
 * The watchdog_ops structure contains a list of low-level operations
 * that control a watchdog device. The start function is mandatory, all other
 * functions are optional.
 */
typedef struct watchdog_ops {
	/* mandatory operations */
	int32_t (*start)(struct watchdog_device *);
	/* optional operations */
	int32_t (*stop)(struct watchdog_device *);
	int32_t (*ping)(struct watchdog_device *);
	int32_t (*set_timeout)(struct watchdog_device *, uint32_t);
	uint32_t (*get_timeleft)(struct watchdog_device *);
} watchdog_ops_t;

/** struct watchdog_device - The structure that defines a watchdog device
 *
 * @ops: Pointer to the list of watchdog operations.
 * @priv: Pointer to the drivers private data.
 */
typedef struct watchdog_device {
    struct watchdog_ops *ops;
    void                *priv;
} watchdog_device_t;

int32_t watchdog_start(struct watchdog_device *wdd);
int32_t watchdog_stop(struct watchdog_device *wdd);
int32_t watchdog_ping(struct watchdog_device *wdd);
int32_t watchdog_set_timeout(struct watchdog_device *wdd, uint32_t timeout);
int32_t watchdog_get_timeleft(struct watchdog_device *wdd, uint32_t *timeleft);

#endif /* WATCHDOG_H_ */