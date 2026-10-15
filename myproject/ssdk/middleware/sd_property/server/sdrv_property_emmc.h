#ifndef __SDRV_PROPERTY_EMMC_H__
#define __SDRV_PROPERTY_EMMC_H__

#include "sdrv_property_list.h"

// property
typedef struct {
    char name[PROP_NAME_MAX];
    char val[PROP_VALUE_MAX];
} property_store_info_t;

typedef struct {
    int origin;
    property_store_info_t item;
} property_set_info_t;

void sdrv_property_emmc_init(void);
int sdrv_property_emmc_async_set(property_set_info_t* item);
int sdrv_property_is_emmc(char* name);

#endif // __SDRV_PROPERTY_EMMC_H__