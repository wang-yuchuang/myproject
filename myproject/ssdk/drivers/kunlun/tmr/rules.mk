LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/sdrv_tmr.c \
    $(LOCAL_DIR)/sdrv_tmr_drv.c

include $(MKROOT)/module.mk