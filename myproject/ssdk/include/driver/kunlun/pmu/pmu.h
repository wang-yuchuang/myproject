/**
 * @file pmu.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pmu driver header file
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_PMU_H
#define INCLUDE_PMU_H

#include <types.h>
#include <cmsis_os2.h>

#define PMU_MAX_PWRPIN_NUM          4
#define PMU_MAX_TRIGGER_CHAN_NUM    2

#if 0
typedef struct pmu_dev pmu_dev_t;

typedef struct pmu_bc_ops {
    void (*init)(pmu_dev_t *device, const sdrv_pmu_bc_ctrl_t *cfg);
    void (*deinit)(pmu_dev_t *device);
    void (*filter)(pmu_dev_t *device, const sdrv_pmu_filter_t *filter);
    void (*window)(pmu_dev_t *device, const sdrv_pmu_bc_window_t *window);
} pmu_bc_ops_t;

typedef struct pmu_io_ops {
    uint32_t (*enable)(pmu_dev_t *device, sdrv_pmu_mode_t pmu_mode, uint8_t id,
                       uint8_t enable);
    uint32_t (*delay)(pmu_dev_t *device, sdrv_pmu_mode_t pmu_mode, uint8_t id,
                      uint8_t delay, uint8_t delay_adj);
    uint32_t (*auto_set)(pmu_dev_t *device, sdrv_pmu_mode_t pmu_mode,
                         uint8_t id, uint8_t mode);
    uint32_t (*polarity)(pmu_dev_t *device, uint8_t id, uint8_t polarity);
    uint32_t (*target)(pmu_dev_t *device, sdrv_pmu_mode_t pmu_mode, uint8_t id,
                       uint8_t target);
} pmu_io_ops_t;

typedef struct _pmu_drv_ops {
    pmu_bc_ops_t bc_button;
    pmu_io_ops_t pwr_ctrl;
    pmu_io_ops_t pwr_on;
} pmu_ops_t;

/**
 * @brief PMU low level device.
 */
typedef struct pmu_dev {
    paddr_t base;
    int irq;
    const pmu_ops_t *ops;
    void *priv;
} pmu_dev_t;
#endif


/* pmu device typedef */
typedef struct pmu_dev pmu_dev_t;

/**
 * @brief pmu device operation interface structure
 *
 */
typedef struct pmu_ops {
    int32_t (*init)(pmu_dev_t* dev);
    int32_t (*deinit)(pmu_dev_t* dev);
    int32_t (*get_status)(pmu_dev_t* dev, int32_t* sta);
    int32_t (*change_state)(pmu_dev_t* dev, int32_t sta);
    int32_t (*get_event_source)(pmu_dev_t* dev, uint32_t* allEvtSrc);
    int32_t (*clear_event_source)(pmu_dev_t* dev, uint32_t multiEvt);
    int32_t (*get_triger_status)(pmu_dev_t* dev, int32_t type, int32_t* sta);
    int32_t (*set_pwr_out_status)(pmu_dev_t* dev, int32_t pinId, int32_t lev);
    int32_t (*get_pwr_input_status)(pmu_dev_t* dev, int32_t pinId, int32_t* lev);
    int32_t (*set_por_force_pwrdown)(pmu_dev_t* dev, int32_t enable);

}pmu_ops_t;


/**
 * @brief pmu device
 *
 */
typedef struct pmu_dev {
    addr_t base;
    bool is_init;
    pmu_ops_t* ops;
    osMutexId_t mutex;
    void* priv;
}pmu_dev_t;


/* pmu API declare */

/**
 * @brief pmu init
 *
 * @param[in] dev pmu device
 * @return int32_t
 */
int32_t pmu_init(pmu_dev_t* dev);

/**
 * @brief pmu deinit
 *
 * @param[in] dev pmu device
 * @return int32_t
 */
int32_t pmu_deinit(pmu_dev_t* dev);

/**
 * @brief get pmu status
 *
 * @param[in] dev pmu device
 * @param[out] sta pmu state status
 * @return int32_t
 */
int32_t pmu_get_status(pmu_dev_t* dev, int32_t* sta);

/**
 * @brief pmu fore power down
 *
 * @param[in] dev pmu device
 * @param[in] sta pmu change dest state
 * @return int32_t
 */
int32_t pmu_change_state(pmu_dev_t* dev, int32_t sta);

/**
 * @brief get all event source status, each event state
 * occupies the bit and refers to the event type enumeration
 * of the corresponding platform
 *
 * @param[in] dev pmu device
 * @param[out] allEvtSrc all event state mix
 * @return int32_t
 */
int32_t pmu_get_event_source(pmu_dev_t* dev, uint32_t* allEvtSrc);

/**
 * @brief Clear the event state of one or more specified
 *  events, and use ' | ' connect multiple event
 *  enumerations
 *
 * @param[in] dev pmu device
 * @param[out] multiEvt multi event enum mix
 * @return int32_t
 */
int32_t pmu_clear_event_source(pmu_dev_t* dev, uint32_t multiEvt);

/**
 * @brief get external reset or wakeup trigger and internal
 * request powerdown trigger status
 *
 * @param[in] dev pmu device
 * @param[in] type trigger type referce to pmu_event_type_enum
 * @param[out] sta trigger status
 * @return int32_t
 */
int32_t pmu_get_triger_status(pmu_dev_t* dev, int32_t type, int32_t* sta);

/**
 * @brief set power control pin out status
 *
 * @param[in] dev pmu device
 * @param[in] pinId pinId referce to pmu_pwrCtl_pinId_enum
 * @param[out] lev out status
 * @return int32_t
 */
int32_t pmu_set_pwr_out_status(pmu_dev_t* dev, int32_t pinId, int32_t lev);

/**
 * @brief get power control pin input status
 *
 * @param[in] dev pmu device
 * @param[in] pinId pinId referce to pmu_pwrCtl_pinId_enum
 * @param[out] lev input level
 * @return int32_t
 */
int32_t pmu_get_pwr_input_status(pmu_dev_t* dev, int32_t pinId, int32_t* lev);

/**
 * @brief set por force to powerdown
 *
 * @param[in] dev pmu device
 * @param[in] enable powerdown or powerup
 * @return int32_t
 */
int32_t pmu_set_por_force_pwrdown(pmu_dev_t* dev, int32_t enable);


#endif /* INCLUDE_PMU_H */
/* end of header file pmu.h */