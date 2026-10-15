#include "sd_property.h"

int sdrv_property_get(int property_id, void* name, void* val) { return -1; }
int sdrv_property_set(int property_id, void* name, void* val) { return -1; }
int sdrv_property_wait_timeout(int property_id, void* name, void* expect, uint32_t ms) { return -1; }
int sdrv_property_init(struct sys_property_value *property_table, int num) { return -1; }

int system_property_get(int property_id, int* val) { return -1; }
int system_property_set(int property_id, int val) { return -1; }
int system_property_wait_condition(int property_id, int expect) { return -1; }
int system_property_wait_timeout(int property_id, int expect, uint32_t ms) { return -1; }
int system_property_observe(int property_id, system_on_changed_cb cb, void *data) { return -1; }