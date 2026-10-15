LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)
LVGL_SDRV_DIR_NAME := $(LVGL_DIR)/kunlun/lvgl_sdrv

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LVGL_SDRV_DIR_NAME)

include $(LVGL_SDRV_DIR_NAME)/lvgl_sdrv.mk

include $(MKROOT)/module.mk
