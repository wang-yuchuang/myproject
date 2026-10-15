#ifndef __SDRV_PROPERTY_STORE_H__
#define __SDRV_PROPERTY_STORE_H__

typedef struct _property_res_t {
    const char* property_path;
    int property_size;
    uint32_t get_offset;
} property_res_t;

void sdrv_property_store_init(void);

#endif // __SDRV_PROPERTY_STORE_H__