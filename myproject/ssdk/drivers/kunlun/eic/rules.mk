LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_SEM), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/kunlun_eic.c \
	$(LOCAL_DIR)/sdrv_eic_drv_kunlun.c
endif
include $(MKROOT)/module.mk