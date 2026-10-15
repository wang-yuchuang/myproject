LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/sdrv_eic.c \
    $(LOCAL_DIR)/sdrv_eic_drv.c

include $(MKROOT)/module.mk