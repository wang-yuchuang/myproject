#ifndef __SDRV_PROPERTY_LIST_H__
#define __SDRV_PROPERTY_LIST_H__

#include "sd_property.h"

#define PROP_NAME_MAX   32
#define PROP_VALUE_MAX  92

int sdrv_property_is_persist(char* name);
int is_sdrv_prop(char* name);
int sdrv_check_name(char* name);
int sdrv_check_val(char* val);
int sdrv_property_list_add(struct sys_property_value *property_table, int num);
int sdrv_property_list_getlen(void);
// struct sys_property_value* sdrv_property_list_getlist(void);
int sdrv_property_list_set_val(int property_id, void* name, void* val, int store);
int sdrv_property_list_get_val(int property_id, void* val);
char* get_list_str(int flag, char* id_list, int id_list_len, int cpu_id);
void sdrv_property_list_init(void);
int get_property_fllower(int property_id, int* last);
int find_id_with_name(char* name);
int sdrv_property_add_callback(int property_id, void* name, void* cb, void *data);
int sdrv_property_list_wait_timeout(int property_id, void* name, void* expect, uint32_t ms);

#endif // __SDRV_PROPERTY_LIST_H__