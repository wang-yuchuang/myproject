LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/sdrv_syscnt.c \
    $(LOCAL_DIR)/sdrv_syscnt_drv.c

include $(MKROOT)/module.mk