LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_pvt_drv.c \
	$(LOCAL_DIR)/pvt.c \
	$(LOCAL_DIR)/sdrv_pvt.c

include $(MKROOT)/module.mk